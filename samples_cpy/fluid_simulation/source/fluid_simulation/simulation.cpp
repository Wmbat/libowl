#include <fluid_simulation/render/camera.hpp>
#include <fluid_simulation/simulation.hpp>

#include <fluid_simulation/collision/primitive.hpp>
#include <fluid_simulation/components.hpp>
#include <fluid_simulation/physics/kernel.hpp>

#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>

#include <glm/ext/matrix_transform.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <fluid_simulation/stb_image_write.h>

#include <execution>
#include <future>

namespace chrono = std::chrono;
namespace vi = ranges::views;

struct mesh_data
{
   glm::mat4 model;
   glm::vec3 colour;
};

auto get_main_framebuffers(const render_system& system, util::logger_wrapper logger)
   -> crl::dynamic_array<framebuffer::create_info>
{
   crl::dynamic_array<framebuffer::create_info> infos;

   const auto swap_extent = system.swapchain().extent();
   for (auto& image_view : system.swapchain().image_views())
   {
      infos.append(
         framebuffer::create_info{.device = system.device().logical(),
                                  .attachments = {image_view.get(), system.get_depth_attachment()},
                                  .width = swap_extent.width,
                                  .height = swap_extent.height,
                                  .layers = 1,
                                  .logger = logger});
   }

   return infos;
}

auto main_colour_attachment(vk::Format format) -> vk::AttachmentDescription
{
   return {.format = format,
           .samples = vk::SampleCountFlagBits::e1,
           .loadOp = vk::AttachmentLoadOp::eClear,
           .storeOp = vk::AttachmentStoreOp::eStore,
           .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
           .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
           .initialLayout = vk::ImageLayout::eUndefined,
           .finalLayout = vk::ImageLayout::ePresentSrcKHR};
}

auto offscreen_colour_attachment(cacao::device& device) -> vk::AttachmentDescription
{
   if (auto res = cacao::find_colour_format(device))
   {
      return {.format = res.value(),
              .samples = vk::SampleCountFlagBits::e1,
              .loadOp = vk::AttachmentLoadOp::eClear,
              .storeOp = vk::AttachmentStoreOp::eStore,
              .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
              .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
              .initialLayout = vk::ImageLayout::eUndefined,
              .finalLayout = vk::ImageLayout::eTransferSrcOptimal};
   }

   return {};
}

auto main_depth_attachment(cacao::device& device) -> vk::AttachmentDescription
{
   if (auto val = cacao::find_depth_format(device))
   {
      return {.format = val.value(),
              .samples = vk::SampleCountFlagBits::e1,
              .loadOp = vk::AttachmentLoadOp::eClear,
              .storeOp = vk::AttachmentStoreOp::eDontCare,
              .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
              .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
              .initialLayout = vk::ImageLayout::eUndefined,
              .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal};
   }

   return {};
}

simulation::simulation(const settings& settings) :
   m_logger{"fluid_simulation"},
   m_settings{settings},
   m_window{"Fluid Simulation", 1080, 720},
   m_render_system{check_err(render_system::make({.logger = &m_logger, .p_window = &m_window}))},
   m_shaders{m_render_system, &m_logger},
   m_pipelines{&m_logger},
   m_main_pipeline_key{},
   m_sphere{create_renderable(m_render_system, load_obj("resources/meshes/sphere.obj"))},
   m_box{create_renderable(m_render_system, load_obj("resources/meshes/box.obj"))}
{
   m_image_pixels.resize(sizeof(glm::u8vec4) * image_height * image_width);

   check_err(m_shaders.insert(m_vert_shader_key, vkn::shader_type::vertex));
   check_err(m_shaders.insert(m_frag_shader_key, vkn::shader_type::fragment));

   m_render_passes.append(render_pass{
      {.device = m_render_system.device().logical(),
       .swapchain = m_render_system.swapchain().value(),
       .colour_attachment = main_colour_attachment(m_render_system.swapchain().format()),
       .depth_stencil_attachment = main_depth_attachment(m_render_system.device()),
       .framebuffer_create_infos = get_main_framebuffers(m_render_system, &m_logger),
       .logger = &m_logger}});

   m_main_pipeline_key = create_main_pipeline();

   m_camera = setup_onscreen_camera(m_main_pipeline_key);

   setup_offscreen();

   glm::vec2 x_edges = {5.0f, -5.0f}; // NOLINT
   glm::vec2 y_edges = {20.0f, 0.0f}; // NOLINT
   glm::vec2 z_edges = {5.0f, -5.0f}; // NOLINT

   {
      glm::vec3 halfs = {half(x_edges.x - x_edges.y),  // NOLINT
                         half(y_edges.x - y_edges.y),  // NOLINT
                         half(z_edges.x - z_edges.y)}; // NOLINT

      float h = m_settings.kernel_radius();
      m_sph_system = sph::system{{.p_registry = util::make_non_null(&m_registry),
                                  .p_logger = util::make_non_null(&m_logger),
                                  .center = halfs,
                                  .dimensions = halfs + h,
                                  .system_settings = settings}};

      m_collision_system = collision::system{{.p_registry = util::make_non_null(&m_registry),
                                              .p_sph_system = util::make_non_null(&m_sph_system)}};
   }

   constexpr std::size_t x_count = 25u;
   constexpr std::size_t y_count = 35u; // 100u;
   constexpr std::size_t z_count = 30u;

   m_particles.reserve(x_count * y_count * z_count);

   float distance_x = settings.water_radius * 1.20f; // NOLINT
   float distance_y = settings.water_radius * 1.20f; // NOLINT
   float distance_z = settings.water_radius * 1.20f; // NOLINT

   for (auto i : vi::iota(0U, x_count))
   {
      const float x = x_edges.y + 1.0f + distance_x * static_cast<float>(i); // NOLINT

      for (auto j : vi::iota(0U, y_count))
      {
         const float y = 0.5f + distance_y * static_cast<float>(j);

         for (auto k : vi::iota(0U, z_count))
         {
            const float z = (-distance_z * z_count / 2.0f) + distance_z * static_cast<float>(k);

            m_sph_system.emit({.position = {x, y, z},
                               .radius = m_settings.water_radius,
                               .mass = m_settings.water_mass});
         }
      }
   }

   // add_box({3.5f, 2.0f, 2.0f}, {1.0f, 2.5f, 3.0f}, {1.0f, 0.0f, 0.0f});
   // add_box({7.5f, 2.0f, -2.0f}, {1.0f, 2.5f, 3.0f}, {1.0f, 0.0f, 0.0f});

   add_invisible_wall({0.0, -1.5f, 0.0f}, {100.0f, 1.5f, 100.0f});             // NOLINT
   add_invisible_wall({x_edges.x + 1.5f, 0.0f, 0.0f}, {1.5f, 100.0f, 100.0f}); // NOLINT
   add_invisible_wall({x_edges.y - 1.0f, 0.0f, 0.0f}, {1.5f, 100.0f, 100.0f}); // NOLINT
   add_invisible_wall({0.0, 0.0f, z_edges.x + 1.5f}, {100.0f, 100.0f, 1.5f});  // NOLINT
   add_invisible_wall({0.0, 0.0f, z_edges.y - 1.5f}, {100.0f, 100.0f, 1.5f});  // NOLINT

   m_logger.info(
      "Scene settings:\n\t-> particle count = {}\n\t-> particle mass = {}\n\t-> particle radius = "
      "{}\n\t-> kernel radius = {}\n\t-> rest density = {}\n\t-> viscosity constant = {}\n\t-> "
      "surface tension coefficient = {}\n\t-> time step = {}ms",
      std::size(m_sph_system.particles()), m_settings.water_mass, m_settings.water_radius,
      m_settings.kernel_radius(), m_settings.rest_density, m_settings.viscosity_constant,
      m_settings.surface_tension_coefficient, m_settings.time_step.count());

   std::filesystem::remove_all("frames");
   std::filesystem::create_directory("frames");
}

void simulation::run()
{
   chrono::duration<float, std::milli> delta_time{};
   auto start_time = std::chrono::steady_clock::now();

   while (m_window.is_open())
   {
      m_window.poll_events();

      update();
      render();

      if (m_frame_count.value() >= max_frames)
      {
         m_logger.info("Render finished!");

         return;
      }

      {
         const auto old = start_time;
         start_time = chrono::steady_clock::now();
         delta_time = start_time - old;

         m_logger.debug("frametime = {}", delta_time.count());
      }

      m_time_spent += m_settings.time_step;
   }

   m_render_system.wait();
}

void simulation::update()
{
   m_sph_system.update(m_settings.time_step);
   m_collision_system.update(m_settings.time_step);
}
void simulation::render()
{
   m_max_density =
      ranges::max_element(m_sph_system.particles(), {}, &sph::particle::density)->density;

   if (has_offscreen_render)
   {
      m_render_system.device().logical().waitForFences({m_offscreen.in_flight_fence.get()}, true,
                                                       std::numeric_limits<std::uint64_t>::max());

      const std::string filename = "frames/frame_" + std::to_string(m_frame_count.value()) + ".png";
      m_image_write_fut = std::async(&simulation::write_image_to_disk, this, filename);

      has_offscreen_render = false;
   }

   onscreen_render();

   if (m_time_spent.count() >= m_time_per_frame.count())
   {
      has_offscreen_render = true;

      if (m_frame_count.value() != 0)
      {
         m_image_write_fut.get();
      }

      m_logger.info(
         "Render status {:0>6.2f}%",
         100.0f * (static_cast<float>(m_frame_count.value()) / static_cast<float>(max_frames - 1)));

      offscreen_render();

      // write_image_to_disk(filename);

      m_time_spent = 0ms;
      ++m_frame_count;
   }
}

void simulation::onscreen_render()
{
   const auto image_index = m_render_system.begin_frame();

   {
      auto extent = m_render_system.swapchain().extent();
      m_camera.update(image_index.value(), compute_matrices(extent.width, extent.height));
   }

   m_render_passes.lookup(0).record_render_calls([&](vk::CommandBuffer buffer) {
      auto& pipeline = check_err(m_pipelines.lookup(m_main_pipeline_key)).value();

      buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.value());

      buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0,
                                {m_camera.lookup_set(image_index.value())}, {});

      auto view = m_registry.view<component::render, component::transform>();

      buffer.bindVertexBuffers(0, {m_box.m_vertex_buffer->get()}, {vk::DeviceSize{0}});
      buffer.bindIndexBuffer(m_box.m_index_buffer->get(), 0, vk::IndexType::eUint32);

      for (auto entity : view | vi::filter([&](auto e) {
                            return view.get<component::render>(e).p_mesh == &m_box;
                         }))
      {
         const auto& render = view.get<component::render>(entity);
         const auto& transform = view.get<component::transform>(entity);

         mesh_data md{.model = transform.translate * transform.scale,
                      .colour = render.colour}; // NOLINT

         buffer.pushConstants(pipeline.layout(),
                              pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
                              sizeof(mesh_data) * 1, &md);

         buffer.drawIndexed(static_cast<std::uint32_t>(m_box.m_index_buffer.index_count()), 1, 0, 0,
                            0);
      }

      buffer.bindVertexBuffers(0, {m_sphere.m_vertex_buffer->get()}, {vk::DeviceSize{0}});
      buffer.bindIndexBuffer(m_sphere.m_index_buffer->get(), 0, vk::IndexType::eUint32);

      for (auto& particle : m_sph_system.particles())
      {
         auto scale =
            glm::scale(glm::mat4{1}, glm::vec3{1.0f, 1.0f, 1.0f} * m_settings.scale_factor);
         auto translate = glm::translate(glm::mat4{1}, particle.position);

         glm::vec3 colour = {65 / 255.0f, 105 / 255.0f, 225 / 255.0f}; // NOLINT
         mesh_data md{.model = translate * scale,
                      .colour = colour * (1 - (particle.density / m_max_density))};

         buffer.pushConstants(pipeline.layout(),
                              pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
                              sizeof(mesh_data) * 1, &md);

         buffer.drawIndexed(static_cast<std::uint32_t>(m_sphere.m_index_buffer.index_count()), 1, 0,
                            0, 0);
      }
   });

   m_render_system.render(m_render_passes);

   m_render_system.end_frame();
}
void simulation::offscreen_render()
{
   auto& device = m_render_system.device();

   device.logical().resetCommandPool(m_offscreen.command_pool.value(), {});

   m_offscreen.cam.update(cacao::index_t{0u}, compute_matrices(image_width, image_height));

   std::array<vk::ClearValue, 2> clear_values{};
   clear_values[0].color = {std::array{0.0F, 0.0F, 0.0F, 0.0F}};
   clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

   for (auto cmd : m_offscreen.command_pool.primary_cmd_buffers())
   {
      cmd.begin({.pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr});

      m_offscreen.pass.record_render_calls([&](vk::CommandBuffer buffer) {
         auto& pipeline = check_err(m_pipelines.lookup(m_offscreen_pipeline_key)).value();

         buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.value());

         buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0,
                                   {m_camera.lookup_set(0)}, {});

         auto view = m_registry.view<component::render, component::transform>();

         buffer.bindVertexBuffers(0, {m_box.m_vertex_buffer->get()}, {vk::DeviceSize{0}});
         buffer.bindIndexBuffer(m_box.m_index_buffer->get(), 0, vk::IndexType::eUint32);

         for (auto entity : view | vi::filter([&](auto e) {
                               return view.get<component::render>(e).p_mesh == &m_box;
                            }))
         {
            const auto& render = view.get<component::render>(entity);
            const auto& transform = view.get<component::transform>(entity);

            mesh_data md{.model = transform.translate * transform.scale,
                         .colour = render.colour}; // NOLINT

            buffer.pushConstants(pipeline.layout(),
                                 pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
                                 sizeof(mesh_data) * 1, &md);

            buffer.drawIndexed(static_cast<std::uint32_t>(m_box.m_index_buffer.index_count()), 1, 0,
                               0, 0);
         }

         buffer.bindVertexBuffers(0, {m_sphere.m_vertex_buffer->get()}, {vk::DeviceSize{0}});
         buffer.bindIndexBuffer(m_sphere.m_index_buffer->get(), 0, vk::IndexType::eUint32);

         for (auto& particle : m_sph_system.particles())
         {
            auto scale =
               glm::scale(glm::mat4{1}, glm::vec3{1.0f, 1.0f, 1.0f} * m_settings.scale_factor);
            auto translate = glm::translate(glm::mat4{1}, particle.position);

            glm::vec3 colour = {65 / 255.0f, 105 / 255.0f, 225 / 255.0f}; // NOLINT
            mesh_data md{.model = translate * scale,
                         .colour = colour * (1 - (particle.density / m_max_density))};

            buffer.pushConstants(pipeline.layout(),
                                 pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
                                 sizeof(mesh_data) * 1, &md);

            buffer.drawIndexed(static_cast<std::uint32_t>(m_sphere.m_index_buffer.index_count()), 1,
                               0, 0, 0);
         }
      });

      m_offscreen.pass.submit_render_calls(
         cmd, cacao::index_t{0}, {.extent = {.width = image_width, .height = image_height}},
         clear_values);

      cmd.end();
   }

   const std::array wait_semaphores{m_offscreen.image_available_semaphore.get()};
   const std::array signal_semaphores{m_offscreen.render_finished_semaphore.get()};
   const std::array command_buffers{m_offscreen.command_pool.primary_cmd_buffers().lookup(0)};
   const std::array<vk::PipelineStageFlags, 1> wait_stages{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

   device.logical().resetFences({m_offscreen.in_flight_fence.get()});

   try
   {
      std::uint32_t wait_semaphore_count =
         m_frame_count.value() == 0 ? 0 : static_cast<std::uint32_t>(std::size(wait_semaphores));
      const vk::Semaphore* p_wait_semaphores =
         m_frame_count.value() == 0 ? nullptr : std::data(wait_semaphores);

      const std::array render_submit_infos{
         vk::SubmitInfo{.waitSemaphoreCount = wait_semaphore_count,
                        .pWaitSemaphores = p_wait_semaphores,
                        .pWaitDstStageMask = std::data(wait_stages),
                        .commandBufferCount = std::size(command_buffers),
                        .pCommandBuffers = std::data(command_buffers),
                        .signalSemaphoreCount = std::size(signal_semaphores),
                        .pSignalSemaphores = std::data(signal_semaphores)}};

      const auto gfx_queue = device.get_queue(cacao::queue_flag_bits::graphics).value;
      gfx_queue.submit(render_submit_infos, m_offscreen.in_flight_fence.get());
   }
   catch (const vk::SystemError& err)
   {
      m_logger.error("failed to submit graphics queue for offscreen rendering");

      std::terminate();
   }

   auto buffer = check_err(m_offscreen.command_pool.create_primary_buffer());
   buffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

   std::array copy_regions{vk::BufferImageCopy{
      .bufferOffset = 0,
      .bufferRowLength = image_width,
      .bufferImageHeight = image_height,
      .imageSubresource = m_offscreen.colour.subresource_layers(),
      .imageOffset = {.x = 0, .y = 0, .z = 0},
      .imageExtent = {.width = image_width, .height = image_height, .depth = 1}}};

   buffer->copyImageToBuffer(m_offscreen.colour.value(), vk::ImageLayout::eTransferSrcOptimal,
                             m_offscreen.image_buffer.get(), std::size(copy_regions),
                             std::data(copy_regions));
   buffer->end();

   try
   {
      const std::array copy_command_buffers{buffer.get()};
      const std::array<vk::PipelineStageFlags, 1> copy_wait_stages{
         vk::PipelineStageFlagBits::eTransfer};

      const std::array render_submit_infos{
         vk::SubmitInfo{.waitSemaphoreCount = std::size(signal_semaphores),
                        .pWaitSemaphores = std::data(signal_semaphores),
                        .pWaitDstStageMask = std::data(copy_wait_stages),
                        .commandBufferCount = std::size(copy_command_buffers),
                        .pCommandBuffers = std::data(copy_command_buffers),
                        .signalSemaphoreCount = std::size(wait_semaphores),
                        .pSignalSemaphores = std::data(wait_semaphores)}};

      const auto gfx_queue = device.get_queue(cacao::queue_flag_bits::graphics).value;
      gfx_queue.submit(render_submit_infos, m_offscreen.in_flight_fence.get());
   }
   catch (const vk::SystemError& err)
   {
      m_logger.error("failed to submit graphics queue for image copy");

      std::terminate();
   }
}

void simulation::setup_offscreen()
{
   auto& device = m_render_system.device();

   m_offscreen.colour = {
      {.logger = &m_logger,
       .device = device,
       .formats = {std::begin(cacao::colour_formats), std::end(cacao::colour_formats)},
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
       .memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
       .width = image_width,
       .height = image_height}};
   m_offscreen.depth = {
      {.logger = &m_logger,
       .device = device,
       .formats = {std::begin(cacao::depth_formats), std::end(cacao::depth_formats)},
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
       .memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
       .width = image_width,
       .height = image_height}};
   m_offscreen.pass =
      render_pass{{.device = device.logical(),
                   .colour_attachment = offscreen_colour_attachment(device),
                   .depth_stencil_attachment = main_depth_attachment(device),
                   .framebuffer_create_infos = {framebuffer::create_info{
                      .device = device.logical(),
                      .attachments = {m_offscreen.colour.view(), m_offscreen.depth.view()},
                      .width = image_width,
                      .height = image_height,
                      .layers = 1,
                      .logger = &m_logger}},
                   .logger = &m_logger}};
   m_offscreen_pipeline_key = create_offscreen_pipeline();
   m_offscreen.cam = setup_offscreen_camera(m_offscreen_pipeline_key);
   m_offscreen.command_pool = check_err(
      vkn::command_pool::builder{device, &m_logger}
         .set_queue_family_index(device.get_queue(cacao::queue_flag_bits::graphics).family_index)
         .set_primary_buffer_count(1U)
         .build());
   m_offscreen.in_flight_fence = m_render_system.device().logical().createFenceUnique(
      vk::FenceCreateInfo{}.setFlags(vk::FenceCreateFlagBits::eSignaled));
   m_offscreen.render_finished_semaphore =
      m_render_system.device().logical().createSemaphoreUnique({});
   m_offscreen.image_available_semaphore =
      m_render_system.device().logical().createSemaphoreUnique({});
   m_offscreen.image_buffer =
      cacao::vulkan::buffer{{.device = device,
                             .buffer_size = sizeof(glm::u8vec4) * image_width * image_height,
                             .usage = vk::BufferUsageFlagBits::eTransferDst,
                             .desired_mem_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                vk::MemoryPropertyFlagBits::eHostCoherent,
                             .logger = &m_logger}};
}

void simulation::add_invisible_wall(const glm::vec3& position, const glm::vec3& dimensions)
{
   auto entity = m_registry.create();
   m_registry.emplace<collision::component::box_collider>(
      entity, collision::component::box_collider{.center = position, .half_size = dimensions});
}
void simulation::add_box(const glm::vec3& position, const glm::vec3& dimensions,
                         const glm::vec3& colour)
{
   auto entity = m_registry.create();
   m_registry.emplace<component::render>(entity,
                                         component::render{.p_mesh = &m_box, .colour = colour});
   m_registry.emplace<collision::component::box_collider>(
      entity, collision::component::box_collider{.center = position, .half_size = dimensions});
   m_registry.emplace<component::transform>(
      entity,
      component::transform{.translate = glm::translate(glm::mat4{1}, position),
                           .scale = glm::scale(glm::mat4{1}, dimensions)});
}
auto simulation::create_main_pipeline() -> pipeline_index_t
{
   auto vert_shader_info = check_err(m_shaders.lookup(m_vert_shader_key));
   auto frag_shader_info = check_err(m_shaders.lookup(m_frag_shader_key));

   const pipeline_shader_data vertex_shader_data{
      .p_shader = &vert_shader_info.value(),
      .set_layouts = {{.name = "camera_layout",
                       .bindings = {{.binding = 0,
                                     .descriptor_type = vk::DescriptorType::eUniformBuffer,
                                     .descriptor_count = 1}}}},
      .push_constants = {{.name = "mesh_data", .size = sizeof(mesh_data), .offset = 0}}};

   const pipeline_shader_data fragment_shader_data{.p_shader = &frag_shader_info.value()};

   crl::dynamic_array<vk::Viewport> pipeline_viewports;
   pipeline_viewports.append(m_render_system.viewport());

   crl::dynamic_array<vk::Rect2D> pipeline_scissors;
   pipeline_scissors.append(m_render_system.scissor());

   crl::dynamic_array<pipeline_shader_data> pipeline_shader_data;
   pipeline_shader_data.append(vertex_shader_data);
   pipeline_shader_data.append(fragment_shader_data);

   auto info = check_err(m_pipelines.insert({.device = m_render_system.device(),
                                             .pass = m_render_passes.lookup(0),
                                             .logger = &m_logger,
                                             .bindings = m_render_system.vertex_bindings(),
                                             .attributes = m_render_system.vertex_attributes(),
                                             .viewports = pipeline_viewports,
                                             .scissors = pipeline_scissors,
                                             .shader_infos = pipeline_shader_data}));

   return info.key();
}
auto simulation::create_offscreen_pipeline() -> pipeline_index_t
{
   auto vert_shader_info = check_err(m_shaders.lookup(m_vert_shader_key));
   auto frag_shader_info = check_err(m_shaders.lookup(m_frag_shader_key));

   const pipeline_shader_data vertex_shader_data{
      .p_shader = &vert_shader_info.value(),
      .set_layouts = {{.name = "camera_layout",
                       .bindings = {{.binding = 0,
                                     .descriptor_type = vk::DescriptorType::eUniformBuffer,
                                     .descriptor_count = 1}}}},
      .push_constants = {{.name = "mesh_data", .size = sizeof(mesh_data), .offset = 0}}};

   const pipeline_shader_data fragment_shader_data{.p_shader = &frag_shader_info.value()};

   crl::dynamic_array<vk::Viewport> pipeline_viewports;
   pipeline_viewports.append(vk::Viewport{.x = 0.0F,
                                          .y = 0.0F,
                                          .width = static_cast<float>(image_width),
                                          .height = static_cast<float>(image_height),
                                          .minDepth = 0.0F,
                                          .maxDepth = 1.0F});

   crl::dynamic_array<vk::Rect2D> pipeline_scissors;
   pipeline_scissors.append(vk::Rect2D{.offset = {0, 0}, .extent = {image_width, image_height}});

   crl::dynamic_array<pipeline_shader_data> pipeline_shader_data;
   pipeline_shader_data.append(vertex_shader_data);
   pipeline_shader_data.append(fragment_shader_data);

   auto info = check_err(m_pipelines.insert({.device = m_render_system.device(),
                                             .pass = m_offscreen.pass,
                                             .logger = &m_logger,
                                             .bindings = m_render_system.vertex_bindings(),
                                             .attributes = m_render_system.vertex_attributes(),
                                             .viewports = pipeline_viewports,
                                             .scissors = pipeline_scissors,
                                             .shader_infos = pipeline_shader_data}));

   return info.key();
}

auto simulation::setup_onscreen_camera(pipeline_index_t index) -> camera
{
   auto pipeline_info = check_err(m_pipelines.lookup(index));

   return create_camera(m_render_system, pipeline_info.value(), &m_logger);
}
auto simulation::setup_offscreen_camera(pipeline_index_t index) -> camera
{
   auto pipeline_info = check_err(m_pipelines.lookup(index));

   return create_offscreen_camera(m_render_system, pipeline_info.value(), &m_logger);
}

auto simulation::compute_matrices(std::uint32_t width, std::uint32_t height) -> camera::matrices
{
   camera::matrices matrices{};
   matrices.projection =
      glm::perspective(glm::radians(90.0F), (float)width / (float)height, 0.1F, 1000.0F); // NOLINT
   matrices.view =
      glm::lookAt(glm::vec3(10.0f, 8.0f, 15.0f), glm::vec3(3.0f, 2.0f, -1.0f), // NOLINT
                  glm::vec3(0.0F, 1.0F, 0.0F));
   matrices.projection[1][1] *= -1;

   return matrices;
}

void simulation::write_image_to_disk(std::string_view name)
{
   auto device = m_render_system.device().logical();

   void* p_data = device.mapMemory(m_offscreen.image_buffer.memory(), 0,
                                   sizeof(glm::u8vec4) * image_height * image_width, {});
   memcpy(std::data(m_image_pixels), p_data, std::size(m_image_pixels));
   device.unmapMemory(m_offscreen.image_buffer.memory());

   stbi_write_png(name.data(), image_width, image_height, 4, std::data(m_image_pixels),
                  image_width * 4);
}

