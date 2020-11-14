#include <water_simulation/camera.hpp>
#include <water_simulation/particle_system.hpp>
#include <water_simulation/pipeline_codex.hpp>
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

#include <numbers>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

static constexpr float pi = std::numbers::pi_v<float>;
static constexpr float time_step = 0.0008F;
static constexpr float rest_density = 1.0F;
static constexpr float gas_constant = 2.0F;
static constexpr float viscosity_constant = 25.0F;
static constexpr float gravity = -9.81F;
static constexpr float gravity_multiplier = 1000.0F;
static constexpr float scale_factor = 0.1F;
static constexpr float water_mass = 1.0F;
static constexpr float kernel_radius = 5.0F;

template <typename Any>
auto handle_err(Any&& result, const std::shared_ptr<util::logger>& p_logger)
{
   if (auto err = result.error())
   {
      util::log_error(p_logger, "error: {}", err->value().message());

      std::exit(EXIT_FAILURE);
   }

   return std::forward<Any>(result).value().value();
}

auto create_camera(render_system& system, vkn::graphics_pipeline& pipeline,
                   const std::shared_ptr<util::logger>& p_logger) -> camera;

auto compute_matrices(const render_system& system) -> camera::matrices
{
   auto dimensions = system.scissor().extent;

   camera::matrices matrices{};
   matrices.projection = glm::perspective(glm::radians(90.0F),
                                          dimensions.width / (float)dimensions.height, 0.1F, 10.0F);
   matrices.view = glm::lookAt(glm::vec3(0.0F, 0.0F, -7.0F), glm::vec3(0.0F, 0.0F, 0.0F),
                               glm::vec3(0.0F, 1.0F, 0.0F));
   matrices.projection[1][1] *= -1;

   return matrices;
}

auto poly6_kernel(float r_squared, float kernel_radius) -> float;
auto spiky_kernel(float r, float kernel_radius) -> float;
auto viscosity_kernel(float r, float kernel_radius) -> float;

void compute_density(std::span<particle_engine::particle> particle);
void compute_forces(std::span<particle_engine::particle> particles);
void integrate(std::span<particle_engine::particle> particles);

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("water_simulation");

   glfwInit();

   ui::window window{"Water Simulation", 1080, 720};

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

   const vkn::pipeline_shader_data vertex_shader_data{
      .p_shader = &vert_shader_info.value(),
      .set_layouts = {{.name = "camera_layout",
                       .bindings = {{.binding = 0,
                                     .descriptor_type = vk::DescriptorType::eUniformBuffer,
                                     .descriptor_count = 1}}}},
      .push_constants = {{.name = "mesh_data", .size = sizeof(glm::mat4), .offset = 0}}};

   const vkn::pipeline_shader_data fragment_shader_data{.p_shader = &frag_shader_info.value()};

   util::dynamic_array<vk::Viewport> pipeline_viewports;
   pipeline_viewports.emplace_back(renderer.viewport());

   util::dynamic_array<vk::Rect2D> pipeline_scissors;
   pipeline_scissors.emplace_back(renderer.scissor());

   util::dynamic_array<vkn::pipeline_shader_data> pipeline_shader_data;
   pipeline_shader_data.push_back(vertex_shader_data);
   pipeline_shader_data.push_back(fragment_shader_data);

   auto main_pipeline_info =
      handle_err(pipeline_codex.insert({.device = renderer.device(),
                                        .render_pass = renderer.render_pass(),
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

   const float distance_x = 3.0F;
   const float distance_y = 3.0F;
   for (auto i : ranges::views::iota(0U, 15U))
   {
      const float x = -distance_x * (15.0F / distance_x) + distance_x * i;

      for (auto j : ranges::views::iota(0U, 15U))
      {
         const float y = -distance_y * (15.0F / distance_x) + distance_y * j;

         particle_engine.emit({.position = {x, y, 0.0F}, .mass = water_mass});
      }
   }

   std::chrono::duration<float, std::ratio<1, 1000>> time_spent{}; // NOLINT
   auto start_time = std::chrono::steady_clock::now();

   while (window.is_open())
   {
      const auto old = start_time;
      start_time = std::chrono::steady_clock::now();
      const decltype(time_spent) delta_time = start_time - old;
      time_spent += delta_time;

      window.poll_events();

      auto active_particles = particle_engine.particles();

      if (time_spent.count() >= 100.0f)
      {
         compute_density(active_particles);
         compute_forces(active_particles);
         integrate(active_particles);

         time_spent = {};
      }

      const auto image_index = renderer.begin_frame();

      camera.update(image_index.value(), compute_matrices(renderer));

      renderer.record_draw_calls([&](vk::CommandBuffer buffer) {
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

      renderer.end_frame();
   }

   renderer.wait();

   return 0;
}

auto poly6_kernel(float r_squared, float kernel_radius) -> float
{
   const float kernel_squared = std::pow(kernel_radius, 2.0F);

   if (r_squared < kernel_squared)
   {
      const float predicate = 315.0F / (65.0F * pi * std::pow(kernel_radius, 9.0F));
      return predicate * std::pow(kernel_squared - r_squared, 3.0F);
   }

   return 0;
}
auto spiky_kernel(float r, float kernel_radius) -> float
{
   if (r < kernel_radius)
   {
      const float predicate = 15.0F / pi * std::pow(kernel_radius, 6.0F);
      return predicate * std::pow(kernel_radius - r, 2.0F);
   }

   return 0;
}
auto viscosity_kernel(float r, float kernel_radius) -> float
{
   if (r < kernel_radius)
   {
      const float predicate = 45.0F / (pi * std::pow(kernel_radius, 6.0f));
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
         const auto r_squared = std::pow(glm::length(particle_j.position - particle_i.position), 2);

         particle_i.density += particle_j.mass * poly6_kernel(r_squared, kernel_radius);
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
            const auto r_ij = particle_j.position - particle_i.position;
            const auto r = glm::length(r_ij);

            pressure_force += -glm::normalize(r_ij) * particle_j.mass *
               (particle_i.position + particle_j.position) / (2.0f * particle_j.density) *
               spiky_kernel(r, kernel_radius);

            viscosity_force += viscosity_constant * particle_j.mass *
               (particle_j.velocity - particle_i.velocity) / particle_j.density *
               viscosity_kernel(r, kernel_radius);
         }
      }

      const auto gravity_force = gravity_vector * particle_i.density;

      particle_i.force = viscosity_force + gravity_force;
   }
}

void integrate(std::span<particle_engine::particle> particles)
{
   for (auto& particle : particles)
   {
      particle.velocity += time_step * particle.force / particle.density;
      particle.position += time_step * particle.velocity;

      if (particle.position.y < -20.0F)
      {
         particle.velocity.y *= -0.5F;
         particle.position.y = -20.0F;
      }

      /*
      util::log_debug(p_logger,
                      "particle\n\t-> position = ({}, {})\n\t-> velocity = ({}, {})\n\t-> force = "
                      "({}, {})\n\t-> density = {}\n\t-> pressure = {}",
                      particle.position.x, particle.position.y, particle.velocity.x,
                      particle.velocity.y, particle.force.x, particle.force.y, particle.density,
                      particle.pressure);
                      */
   }
};

auto create_camera(render_system& system, vkn::graphics_pipeline& pipeline,
                   const std::shared_ptr<util::logger>& p_logger) -> camera
{
   auto& config = system.lookup_configuration();

   return handle_err(camera::make({.renderer = system,
                                   .pipeline = pipeline,
                                   .image_count = config.swapchain_image_count,
                                   .p_logger = p_logger}),
                     p_logger);
}
