#include <sph-simulation/simulation.hpp>

#include <sph-simulation/components.hpp>
#include <sph-simulation/sim_variables.hpp>
#include <sph-simulation/transform.hpp>

#include <sph-simulation/core/pipeline.hpp>
#include <sph-simulation/core/pipeline_registry.hpp>
#include <sph-simulation/core/shader_registry.hpp>

#include <sph-simulation/physics/collision/colliders.hpp>
#include <sph-simulation/physics/rigid_body.hpp>
#include <sph-simulation/physics/system.hpp>

#include <sph-simulation/sph/system.hpp>

#include <sph-simulation/render/core/camera.hpp>
#include <sph-simulation/render/frame_manager.hpp>

#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>

#include <glm/ext/matrix_transform.hpp>

#if defined(__GNUC__) || defined(__clang__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#   pragma GCC diagnostic ignored "-Wsign-compare"
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#if defined(__GNUC__) || defined(__clang__)
#   pragma GCC diagnostic pop
#endif

#include <execution>
#include <future>

namespace vi = ranges::views;

using mannele::u32;
using mannele::u64;

using namespace reglisse;

struct mesh_data
{
   glm::mat4 model;
   glm::vec3 colour;
};

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

auto main_depth_attachment(cacao::device& device) -> vk::AttachmentDescription
{
   if (auto val = find_depth_format(device))
   {
      return {.format = val.borrow(),
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

auto create_window(const sim_config& config) -> maybe<cacao::window>;
auto create_render_command_pools(const cacao::device& device, mannele::log_ptr logger)
   -> std::array<cacao::command_pool, max_frames_in_flight>;
auto compute_matrices(const vk::Extent2D& extent) -> camera::matrices;
void setup_particles(entt::registry& registry, const sim_variables& variables,
                     const renderable& renderable);

struct render_pass_data
{
   render_pass pass;

   vk::Rect2D render_area;
   std::array<vk::ClearValue, 2> clear_values;
};

struct update_info
{
   entt::registry& registry;

   const sim_variables& variables;
   duration<float> time_step;
};

struct render_info
{
   cacao::device& device;

   frame_manager& frame_man;
   std::span<cacao::command_pool> pools;

   std::span<render_pass_data> render_passes;

   camera& main_camera;

   entt::registry& registry;
};

void update(const update_info& info);
void render(const render_info& info);

auto start_simulation(const simulation_info& info) -> int
{
   auto logger = info.logger;
   auto window = cacao::window(
      {.title = info.config.name, .dimension = info.config.dimensions, .is_resizable = false});
   auto context =
      cacao::context({.min_vulkan_version = VK_MAKE_VERSION(1, 0, 0), .logger = logger});
   auto surface = window.create_surface(context).take();
   auto device = cacao::device(
      {.ctx = context, .surface = surface.get(), .use_transfer_queue = true, .logger = logger});

   std::array render_command_pools = create_render_command_pools(device, logger);
   auto transfer_pool = cacao::command_pool(cacao::command_pool_create_info{
      .device = device,
      .queue_family_index =
         some(device.find_best_suited_queue(cacao::queue_flag_bits::transfer).family_index),
      .primary_buffer_count = 0,
      .secondary_buffer_count = 0,
      .logger = logger});

   auto shaders = shader_registry(device, logger);
   shaders.insert("shaders/test_vert.spv", cacao::shader_type::vertex);
   shaders.insert("shaders/test_frag.spv", cacao::shader_type::fragment);

   entt::registry entity_registry;

   std::vector<renderable> renderables;
   renderables.push_back(create_renderable(
      device, transfer_pool, load_obj(asset_default_dir / "meshes/sphere.obj"), logger));
   renderables.push_back(create_renderable(
      device, transfer_pool, load_obj(asset_default_dir / "meshes/cube.obj"), logger));

   auto frame_man = frame_manager({.window = window,
                                   .device = device,
                                   .surface = surface.get(),
                                   .image_usage = vk::ImageUsageFlagBits::eColorAttachment |
                                      vk::ImageUsageFlagBits::eTransferSrc,
                                   .logger = logger});

   auto pipelines = pipeline_registry(logger);

   std::array<vk::ClearValue, 2> clear_values{};
   clear_values[0].color = {std::array{0.0F, 0.0F, 0.0F, 0.0F}};
   clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

   std::vector<render_pass_data> render_passes;
   render_passes.push_back(
      {.pass =
          render_pass({.device = device,
                       .colour_attachment = some(main_colour_attachment(frame_man.frame_format())),
                       .depth_stencil_attachment = some(main_depth_attachment(device)),
                       .framebuffer_create_infos = frame_man.get_framebuffer_info(),
                       .logger = logger}),
       .render_area = {.offset = {0, 0}, .extent = frame_man.extent()},
       .clear_values = clear_values});

   pipeline_registry::key_type main_pipeline_key = 0;
   {
      std::vector viewports = {vk::Viewport{.x = 0.0F,
                                            .y = 0.0F,
                                            .width = static_cast<float>(frame_man.extent().width),
                                            .height = static_cast<float>(frame_man.extent().height),
                                            .minDepth = 0.0F,
                                            .maxDepth = 1.0F}};

      std::vector scissors = {vk::Rect2D{.offset = {0, 0}, .extent = frame_man.extent()}};

      auto vert_shader_info = shaders.lookup("shaders/test_vert.spv").borrow();
      auto frag_shader_info = shaders.lookup("shaders/test_frag.spv").borrow();
      std::vector shader_data = {
         pipeline_shader_data{
            .p_shader = &vert_shader_info.value(),
            .set_layouts = {{.name = "camera_layout",
                             .bindings = {{.binding = 0,
                                           .descriptor_type = vk::DescriptorType::eUniformBuffer,
                                           .descriptor_count = 1}}}},
            .push_constants = {{.name = "mesh_data", .size = sizeof(mesh_data), .offset = 0}}},
         pipeline_shader_data{.p_shader = &frag_shader_info.value()}};

      std::vector bindings = {vk::VertexInputBindingDescription{
         .binding = 0, .stride = sizeof(vertex), .inputRate = vk::VertexInputRate::eVertex}};

      std::vector attributes = {
         vk::VertexInputAttributeDescription{.location = 0,
                                             .binding = 0,
                                             .format = vk::Format::eR32G32B32Sfloat,
                                             .offset = offsetof(vertex, position)},
         vk::VertexInputAttributeDescription{.location = 1,
                                             .binding = 0,
                                             .format = vk::Format::eR32G32B32Sfloat,
                                             .offset = offsetof(vertex, normal)},
         vk::VertexInputAttributeDescription{.location = 2,
                                             .binding = 0,
                                             .format = vk::Format::eR32G32B32Sfloat,
                                             .offset = offsetof(vertex, colour)}};

      auto insertion_result = pipelines.insert({.device = device,
                                                .pass = render_passes.at(0).pass,
                                                .logger = logger,
                                                .bindings = bindings,
                                                .attributes = attributes,
                                                .viewports = viewports,
                                                .scissors = scissors,
                                                .shader_infos = shader_data});

      if (insertion_result.is_ok())
      {
         main_pipeline_key = insertion_result.borrow().key();
      }
      else
      {
         logger.error("Failed to create main rendering pipeline");
         logger.error("Application cannot proceed forward. Shutting down...");

         return EXIT_FAILURE;
      }
   }

   auto& main_pipeline =
      pipelines.lookup<pipeline_type::graphics>(main_pipeline_key).borrow().value();
   auto main_camera = camera({.device = device,
                              .layout = main_pipeline.get_descriptor_set_layout("camera_layout"),
                              .image_count = static_cast<u32>(frame_man.image_count()),
                              .logger = logger});

   render_passes[0].pass.record_render_calls([&](vk::CommandBuffer buffer, u64 image_index) {
      auto& pipeline =
         pipelines.lookup<pipeline_type::graphics>(main_pipeline_key).borrow().value();

      buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.value());

      buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0,
                                {main_camera.lookup_set(image_index)}, {});

      auto view = entity_registry.view<component::mesh, transform>();

      for (auto& renderable : renderables)
      {
         buffer.bindVertexBuffers(0, {renderable.vertex_buff.buffer().value()},
                                  {vk::DeviceSize{0}});
         buffer.bindIndexBuffer(renderable.index_buff.buffer().value(), 0, vk::IndexType::eUint32);

         for (auto entity : view | vi::filter([&](auto e) {
                               return view.get<component::mesh>(e).p_mesh == &renderable;
                            }))
         {
            const auto& render = view.get<component::mesh>(entity);
            const auto& transform = view.get<::transform>(entity);

            const auto translate = glm::translate(glm::mat4(1), transform.position);
            const auto scale = glm::scale(glm::mat4(1), transform.scale);

            mesh_data md{.model = translate * scale, .colour = render.colour}; // NOLINT

            buffer.pushConstants(pipeline.layout(),
                                 pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
                                 sizeof(mesh_data) * 1, &md);

            buffer.drawIndexed(static_cast<std::uint32_t>(renderable.index_buff.index_count()), 1,
                               0, 0, 0);
         }
      }
   });

   setup_particles(entity_registry, info.config.variables, renderables[0]);

   logger.info("Starting render...");

   u32 current_frame = 0;
   while (current_frame < info.config.frame_count)
   {
      update({.registry = entity_registry,
              .variables = info.config.variables,
              .time_step = info.config.time_step});
      render({.device = device,
              .frame_man = frame_man,
              .pools = render_command_pools,
              .render_passes = render_passes,
              .main_camera = main_camera,
              .registry = entity_registry});

      ++current_frame;

      const float completion_rate =
         static_cast<float>(current_frame) / static_cast<float>(info.config.frame_count);
      logger.info("Render status: {:0>6.2f}%", 100.0f * completion_rate);
   }

   logger.info("Render Finished");
   logger.info("Closing program...");

   device.logical().waitIdle();

   return EXIT_SUCCESS;
}

void update(const update_info& info)
{
   const auto particle_view = info.registry.view<PARTICLE_COMPONENTS>();
   const auto sphere_view = info.registry.view<SPHERE_COMPONENTS>();
   const auto plane_view = info.registry.view<PLANE_COMPONENTS>();
   const auto box_view = info.registry.view<BOX_COMPONENTS>();

   sph::update({.particles = particle_view,
                .spheres = sphere_view,
                .planes = plane_view,
                .boxes = box_view,
                .variables = info.variables,
                .time_step = info.time_step});
   physics::update({.spheres = sphere_view,
                    .planes = plane_view,
                    .boxes = box_view,
                    .time_step = info.time_step});
}
void render(const render_info& info)
{
   auto device = info.device.logical();
   auto& main_camera = info.main_camera;

   const auto [image_index, frame_index] = info.frame_man.begin_frame().take();

   main_camera.update(image_index, compute_matrices(info.frame_man.extent()));
   device.resetCommandPool(info.pools[frame_index].value(), {});

   for (auto& buffer : info.pools[frame_index].primary_buffers())
   {
      buffer.begin(vk::CommandBufferBeginInfo{});

      for (auto& render_pass : info.render_passes)
      {
         render_pass.pass.submit_render_calls(buffer, image_index, render_pass.render_area,
                                              render_pass.clear_values);
      }

      buffer.end();
   }

   info.frame_man.end_frame(info.pools);
}

auto create_render_command_pools(const cacao::device& device, mannele::log_ptr logger)
   -> std::array<cacao::command_pool, max_frames_in_flight>
{
   std::array<cacao::command_pool, max_frames_in_flight> pools;

   for (auto& pool : pools)
   {
      const cacao::queue desired_queue = device.find_best_suited_queue(
         cacao::queue_flag_bits::graphics | cacao::queue_flag_bits::present);

      pool = cacao::command_pool({.device = device,
                                  .queue_family_index = some(desired_queue.family_index),
                                  .primary_buffer_count = 1,
                                  .logger = logger});
   }

   return pools;
}
auto compute_matrices(const vk::Extent2D& extent) -> camera::matrices
{
   const auto width = static_cast<float>(extent.width);
   const auto height = static_cast<float>(extent.height);

   camera::matrices matrices{};
   matrices.projection =
      glm::perspective(glm::radians(90.0F), width / height, 0.1F, 1000.0F); // NOLINT
   matrices.view =
      glm::lookAt(glm::vec3(10.0f, 8.0f, 15.0f), glm::vec3(3.0f, 2.0f, -1.0f), // NOLINT
                  glm::vec3(0.0F, 1.0F, 0.0F));
   matrices.projection[1][1] *= -1;

   return matrices;
}
void setup_particles(entt::registry& registry, const sim_variables& variables,
                     const renderable& renderable)
{
   constexpr std::size_t x_count = 10u;
   constexpr std::size_t y_count = 10u; // 100u;
   constexpr std::size_t z_count = 10u;

   float distance_x = variables.water_radius * 1.20f; // NOLINT
   float distance_y = variables.water_radius * 1.20f; // NOLINT
   float distance_z = variables.water_radius * 1.20f; // NOLINT

   for (auto i : vi::iota(0U, x_count))
   {
      const float x = -4.0f + distance_x * static_cast<float>(i); // NOLINT

      for (auto j : vi::iota(0U, y_count))
      {
         const float y = 0.5f + distance_y * static_cast<float>(j);

         for (auto k : vi::iota(0U, z_count))
         {
            const float z = (-distance_z * z_count / 2.0f) + distance_z * static_cast<float>(k);

            auto entity = registry.create();

            auto& transform = registry.emplace<::transform>(entity);
            transform = {.position = {x, y, z},
                         .rotation = {0, 0, 0},
                         .scale = glm::vec3(1.0f, 1.0f, 1.0f) * 0.25f}; // NOLINT

            auto& particle = registry.emplace<sph::particle>(entity);
            particle = {.radius = variables.water_radius, .mass = variables.water_mass};

            auto& mesh = registry.emplace<component::mesh>(entity);
            mesh = {.p_mesh = &renderable,
                    .colour = {65 / 255.0f, 105 / 255.0f, 225 / 255.0f}}; // NOLINT

            auto& collider = registry.emplace<physics::sphere_collider>(entity);
            collider = {.volume = {.center = glm::vec3(), .radius = variables.water_radius},
                        .friction = 0.0f,
                        .restitution = 0.5f}; // NOLINT
         }
      }
   }
}
