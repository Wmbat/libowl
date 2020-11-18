#include <water_simulation/camera.hpp>
#include <water_simulation/particle_system.hpp>
#include <water_simulation/pipeline_codex.hpp>
#include <water_simulation/render_pass.hpp>
#include <water_simulation/render_system.hpp>
#include <water_simulation/renderable.hpp>
#include <water_simulation/shader_codex.hpp>

#include <gfx/data_types.hpp>
#include <gfx/render_manager.hpp>
#include <gfx/window.hpp>

#include <ui/window.hpp>

#include <glm/ext/matrix_transform.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>

#include <cmath>
#include <numbers>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace chrono = std::chrono;

static constexpr float pi = std::numbers::pi_v<float>;
static constexpr float time_step = 0.016f;
static constexpr float rest_density = 2.0f;
static constexpr float gas_constant = 2.0f;
static constexpr float viscosity_constant = 0.1f;
static constexpr float gravity = -9.81f;
static constexpr float gravity_multiplier = 1.0f;
static constexpr float scale_factor = 0.05f;
static constexpr float water_mass = 2.0f;
static constexpr float kernel_radius = 6.0f;
static constexpr float bound_damping = 0.25f;

auto compute_matrices(const render_system& system) -> camera::matrices
{
   auto dimensions = system.scissor().extent;

   camera::matrices matrices{};
   matrices.projection = glm::perspective(
      glm::radians(90.0F), dimensions.width / (float)dimensions.height, 0.1F, 100.0F);    // NOLINT
   matrices.view = glm::lookAt(glm::vec3(0.3F, 1.0F, -2.0F), glm::vec3(0.3F, 1.0F, 0.0F), // NOLINT
                               glm::vec3(0.0F, 1.0F, 0.0F));
   matrices.projection[1][1] *= -1;

   return matrices;
}

auto get_main_framebuffers(const render_system& system, const std::shared_ptr<util::logger>& logger)
   -> util::dynamic_array<framebuffer::create_info>
{
   util::dynamic_array<framebuffer::create_info> infos;

   const auto swap_extent = system.swapchain().extent();
   for (auto& image_view : system.swapchain().image_views())
   {
      infos.emplace_back(
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

auto main_depth_attachment(vkn::device& device) -> vk::AttachmentDescription
{
   if (auto format = find_depth_format(device))
   {
      return {.format = format.value().value(),
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

auto poly6_kernel(float r_squared) -> float;
auto spiky_kernel(float r) -> float;
auto viscosity_kernel(float r) -> float;

void compute_density(std::span<particle_engine::particle> particle);
void compute_forces(std::span<particle_engine::particle> particles);
void integrate(std::span<particle_engine::particle> particles);

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("water_simulation");

   glfwInit();

   ui::window window{"Water Simulation", 1080, 720}; // NOLINT

   render_system renderer =
      handle_err(render_system::make({.p_logger = main_logger, .p_window = &window}), main_logger);

   shader_codex shader_codex{renderer, main_logger};
   pipeline_codex pipeline_codex{main_logger};

   const auto vert_shader_info =
      handle_err(shader_codex.insert("resources/shaders/test_vert.spv", vkn::shader_type::vertex),
                 main_logger);
   const auto frag_shader_info =
      handle_err(shader_codex.insert("resources/shaders/test_frag.spv", vkn::shader_type::fragment),
                 main_logger);

   util::dynamic_array<render_pass> render_passes;

   render_passes.emplace_back(handle_err(
      render_pass::make({.device = renderer.device().logical(),
                         .swapchain = renderer.swapchain().value(),
                         .colour_attachment = main_colour_attachment(renderer.swapchain().format()),
                         .depth_stencil_attachment = main_depth_attachment(renderer.device()),
                         .framebuffer_create_infos = get_main_framebuffers(renderer, main_logger),
                         .logger = main_logger}),
      main_logger));

   const pipeline_shader_data vertex_shader_data{
      .p_shader = &vert_shader_info.value(),
      .set_layouts = {{.name = "camera_layout",
                       .bindings = {{.binding = 0,
                                     .descriptor_type = vk::DescriptorType::eUniformBuffer,
                                     .descriptor_count = 1}}}},
      .push_constants = {{.name = "mesh_data", .size = sizeof(glm::mat4), .offset = 0}}};

   const pipeline_shader_data fragment_shader_data{.p_shader = &frag_shader_info.value()};

   util::dynamic_array<vk::Viewport> pipeline_viewports;
   pipeline_viewports.emplace_back(renderer.viewport());

   util::dynamic_array<vk::Rect2D> pipeline_scissors;
   pipeline_scissors.emplace_back(renderer.scissor());

   util::dynamic_array<pipeline_shader_data> pipeline_shader_data;
   pipeline_shader_data.push_back(vertex_shader_data);
   pipeline_shader_data.push_back(fragment_shader_data);

   auto main_pipeline_info =
      handle_err(pipeline_codex.insert({.device = renderer.device(),
                                        .render_pass = render_passes[0],
                                        .p_logger = main_logger,
                                        .bindings = renderer.vertex_bindings(),
                                        .attributes = renderer.vertex_attributes(),
                                        .viewports = pipeline_viewports,
                                        .scissors = pipeline_scissors,
                                        .shader_infos = pipeline_shader_data}),
                 main_logger);

   auto camera = create_camera(renderer, main_pipeline_info.value(), main_logger);
   auto sphere = create_renderable(renderer, load_obj("resources/meshes/sphere.obj"));

   particle_engine particle_engine{main_logger};

   const float distance_x = 2.0f;
   const float distance_y = 2.0f;
   for (auto i : ranges::views::iota(0U, 15u))
   {
      const float x = (-distance_x * 10.0f / 2.0f) + distance_x * i;

      for (auto j : ranges::views::iota(0U, 100u))
      {
         const float y = 10.0f + distance_y * j;

         particle_engine.emit({.position = {x, y, 0.0f}, .mass = water_mass});
      }
   }

   util::log_info(main_logger, "particle count = {}", std::size(particle_engine.particles()));

   chrono::duration<float, std::milli> time_spent{};
   auto start_time = std::chrono::steady_clock::now();

   while (window.is_open())
   {
      window.poll_events();

      auto active_particles = particle_engine.particles();

      if (time_spent.count() >= time_step)
      {
         compute_density(active_particles);
         compute_forces(active_particles);
         integrate(active_particles);

         time_spent = {};
      }

      const auto image_index = renderer.begin_frame();

      camera.update(image_index.value(), compute_matrices(renderer));

      render_passes[0].record_render_calls([&](vk::CommandBuffer buffer) {
         auto& pipeline = main_pipeline_info.value();

         buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.value());

         buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0,
                                   {camera.lookup_set(image_index.value())}, {});

         buffer.bindVertexBuffers(0, {sphere.m_vertex_buffer->value()}, {vk::DeviceSize{0}});
         buffer.bindIndexBuffer(sphere.m_index_buffer->value(), 0, vk::IndexType::eUint32);

         for (const auto& particle : active_particles)
         {
            const auto model = glm::translate(
               glm::scale(glm::mat4{1}, glm::vec3{scale_factor, scale_factor, scale_factor}),
               particle.position);

            buffer.pushConstants(pipeline.layout(),
                                 pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
                                 sizeof(glm::mat4) * 1, &model);

            buffer.drawIndexed(sphere.m_index_buffer.index_count(), 1, 0, 0, 0);
         }
      });

      renderer.render(render_passes);
      renderer.end_frame();

      const auto old = start_time;
      start_time = chrono::steady_clock::now();
      const chrono::duration<float, std::milli> delta_time = start_time - old;
      time_spent += delta_time;
   }

   renderer.wait();

   return 0;
}

auto poly6_kernel(float r) -> float
{
   if (r <= kernel_radius)
   {
      const float r_squared = std::pow(r, 2.0f);

      constexpr float k_squared = my_pow(kernel_radius, 2.0f);
      constexpr float predicate = 315.0F / (65.0F * pi * my_pow(kernel_radius, 9.0F));

      return predicate * std::pow(k_squared - r_squared, 3.0F); // NOLINT
   }

   return 0;
}
auto spiky_kernel(float r) -> float
{
   if (r <= kernel_radius)
   {
      constexpr float predicate = 15.0f / (pi * my_pow(kernel_radius, 6u));

      return predicate * std::pow(kernel_radius - r, 3.0F); // NOLINT
   }

   return 0;
}
auto viscosity_kernel(float r) -> float
{
   if (r <= kernel_radius)
   {
      constexpr float predicate = 45.0F / (pi * my_pow(kernel_radius, 6.0f));

      return predicate * (kernel_radius - r);
   }

   return 0;
}

void compute_density(std::span<particle_engine::particle> particles)
{
   for (auto& particle_i : particles)
   {
      particle_i.density = 0.0F;

      for (auto& particle_j : particles)
      {
         const auto r_ij = particle_j.position - particle_i.position;
         const auto r = glm::length(r_ij);

         particle_i.density += particle_j.mass * poly6_kernel(r);
      }
   }
}
void compute_forces(std::span<particle_engine::particle> particles)
{
   const glm::vec3 gravity_vector{0.0f, gravity * gravity_multiplier, 0.0f};
   for (auto& particle_i : particles)
   {
      particle_i.pressure = gas_constant * (particle_i.density - rest_density);

      glm::vec3 pressure_force{0.0f, 0.0f, 0.0f};
      glm::vec3 viscosity_force{0.0f, 0.0f, 0.0f};

      for (auto& particle_j : particles)
      {
         if (&particle_i != &particle_j)
         {
            auto r_ij = particle_j.position - particle_i.position;
            if (r_ij.x == 0.0f && r_ij.y == 0.0f) // NOLINT
            {
               r_ij.x += 0.0001f; // NOLINT
               r_ij.y += 0.0001f; // NOLINT
            }

            const auto r = glm::length(r_ij);

            pressure_force += -glm::normalize(r_ij) * particle_j.mass *
               (particle_i.pressure + particle_j.pressure) / (2.0f * particle_j.density) * // NOLINT
               spiky_kernel(r);

            viscosity_force += viscosity_constant * particle_j.mass *
               (particle_j.velocity - particle_i.velocity) / particle_j.density *
               viscosity_kernel(r);
         }
      }

      const auto gravity_force = gravity_vector * particle_i.density;

      particle_i.force = viscosity_force + pressure_force + gravity_force;
   }
}
void integrate(std::span<particle_engine::particle> particles)
{
   for (auto& particle : particles)
   {
      particle.velocity += time_step * particle.force / particle.density;
      particle.position += time_step * particle.velocity;

      if (particle.position.x - kernel_radius < -15.0f) // NOLINT
      {
         particle.velocity.x *= -bound_damping;       // NOLINT
         particle.position.x = kernel_radius - 15.0f; // NOLINT
      }

      if (particle.position.x + kernel_radius > 25.0f) // NOLINT
      {
         particle.velocity.x *= -bound_damping;       // NOLINT
         particle.position.x = 25.0F - kernel_radius; // NOLINT
      }

      if (particle.position.y - kernel_radius < 0.0F) // NOLINT
      {
         particle.velocity.y *= -bound_damping; // NOLINT
         particle.position.y = kernel_radius;   // NOLINT
      }
   }
};
