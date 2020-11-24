#include <water_simulation/simulation.hpp>

#include <water_simulation/collision/primitive.hpp>
#include <water_simulation/components.hpp>
#include <water_simulation/physics/kernel.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>

#include <glm/ext/matrix_transform.hpp>

#include <execution>

namespace chrono = std::chrono;
namespace vi = ranges::views;

struct mesh_data
{
   glm::mat4 model;
   glm::vec3 colour;
};

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
   const auto res = find_depth_format(device);
   if (auto val = res.value())
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

void compute_density(std::span<particle> particles, const settings& settings)
{
   std::transform(std::execution::par, std::begin(particles), std::end(particles),
                  std::begin(particles), [&](const auto& particle_i) {
                     float density = 0.0F;

                     const auto h = settings.kernel_radius();

                     for (auto& particle_j : particles)
                     {
                        const auto r_ij = particle_j.position - particle_i.position;
                        const auto r = glm::length(r_ij);

                        if (r <= h)
                        {
                           density += kernel::poly6(h, r);
                        }
                     }

                     float density_ratio = particle_i.density / settings.rest_density;

                     particle r{particle_i};
                     r.density = density * settings.water_mass * kernel::poly6_constant(h);
                     r.pressure = density_ratio < 1.0f ? 0 : std::pow(density_ratio, 7.0f) - 1.0f;

                     return r;
                  });
}
void compute_normals(std::span<particle> particles, const settings& settings)
{
   std::transform(std::execution::par, std::begin(particles), std::end(particles),
                  std::begin(particles), [&](const auto& particle_i) {
                     glm::vec3 normal{0.0f, 0.0f, 0.0f};

                     const auto h = settings.kernel_radius();

                     for (const auto& particle_j : particles)
                     {
                        const auto r_ij = particle_j.position - particle_i.position;
                        const auto r = glm::length(r_ij);

                        if (r <= h)
                        {
                           normal += kernel::poly6_grad(r_ij, h, r) / particle_j.density;
                        }
                     }

                     particle r = particle_i;
                     r.normal = normal * h * settings.water_radius * kernel::poly6_grad_constant(h);

                     return r;
                  });
}
void compute_forces(std::span<particle> particles, const settings& settings)
{
   const glm::vec3 gravity_vector{0.0f, gravity * settings.gravity_multiplier, 0.0f};

   std::transform(
      std::execution::par, std::begin(particles), std::end(particles), std::begin(particles),
      [&](const auto& particle_i) {
         glm::vec3 pressure_force{0.0f, 0.0f, 0.0f};
         glm::vec3 viscosity_force{0.0f, 0.0f, 0.0f};
         glm::vec3 cohesion_force{0.0f, 0.0f, 0.0f};
         glm::vec3 curvature_force{0.0f, 0.0f, 0.0f};
         glm::vec3 gravity_force{0.0f, 0.0f, 0.0f};

         const float h = settings.kernel_radius();

         for (const auto& particle_j : particles)
         {
            if (&particle_i != &particle_j)
            {
               auto r_ij = particle_i.position - particle_j.position;
               if (r_ij.x == 0.0f && r_ij.y == 0.0f) // NOLINT
               {
                  r_ij.x += 0.0001f; // NOLINT
                  r_ij.y += 0.0001f; // NOLINT
               }

               const auto r = glm::length(r_ij);

               if (r < h)
               {
                  pressure_force -=
                     ((particle_i.pressure + particle_j.pressure) / (2.0f * particle_j.density)) *
                     kernel::spiky_grad(r_ij, h, r);

                  if (particle_j.density > 0.00001f) // NOLINT
                  {
                     viscosity_force -=
                        ((particle_j.velocity - particle_i.velocity) / particle_j.density) *
                        kernel::viscosity(h, r);
                  }

                  const float correction_factor =
                     2.0f * settings.rest_density / (particle_i.density + particle_j.density);

                  cohesion_force += correction_factor * (r_ij / r) * kernel::cohesion(h, r);
                  curvature_force += correction_factor * (particle_i.normal - particle_j.normal);
               }
            }
         }

         gravity_force += gravity_vector * particle_i.density;
         pressure_force *= kernel::spiky_grad_constant(h);
         viscosity_force *= settings.viscosity_constant * kernel::viscosity_constant(h);
         cohesion_force *= -settings.surface_tension_coefficient * settings.water_mass *
            kernel::cohesion_constant(h);
         curvature_force *= -settings.surface_tension_coefficient;

         const auto main_forces =
            (viscosity_force + pressure_force + cohesion_force + curvature_force) *
            settings.water_mass;

         particle r{particle_i};
         r.force = main_forces + gravity_force;

         return r;
      });
}

simulation::simulation(const settings& settings) :
   m_logger{std::make_shared<util::logger>("water_simulation")},
   m_settings{settings}, m_window{"Water Simulation", 1920, 1080}, // NOLINT
   m_render_system{check_err(render_system::make({.p_logger = m_logger, .p_window = &m_window}))},
   m_shaders{m_render_system, m_logger}, m_pipelines{m_logger}, m_main_pipeline_key{},
   m_sphere{create_renderable(m_render_system, load_obj("resources/meshes/sphere.obj"))},
   m_box{create_renderable(m_render_system, load_obj("resources/meshes/box.obj"))}
{
   check_err(m_shaders.insert(m_vert_shader_key, vkn::shader_type::vertex));
   check_err(m_shaders.insert(m_frag_shader_key, vkn::shader_type::fragment));

   m_render_passes.emplace_back(check_err(render_pass::make(
      {.device = m_render_system.device().logical(),
       .swapchain = m_render_system.swapchain().value(),
       .colour_attachment = main_colour_attachment(m_render_system.swapchain().format()),
       .depth_stencil_attachment = main_depth_attachment(m_render_system.device()),
       .framebuffer_create_infos = get_main_framebuffers(m_render_system, m_logger),
       .logger = m_logger})));

   m_main_pipeline_key = create_main_pipeline();
   m_camera = setup_camera(m_main_pipeline_key);

   constexpr std::size_t x_count = 11u;
   constexpr std::size_t y_count = 50u; // 100u;
   constexpr std::size_t z_count = 11u;

   m_particles.reserve(x_count * y_count * z_count);

   float distance_x = settings.water_radius;
   float distance_y = settings.water_radius;
   float distance_z = settings.water_radius;

   for (auto i : vi::iota(0U, x_count))
   {
      const float x = 40.0f + (-distance_x * x_count / 2.0f) + distance_x * i;

      for (auto j : vi::iota(0U, y_count))
      {
         const float y = 5.0f + distance_y * j;

         for (auto k : vi::iota(0U, z_count))
         {
            const float z = (-distance_z * z_count / 2.0f) + distance_z * k;

            m_particles.emplace_back(
               particle{.position = {x, y, z}, .mass = m_settings.water_mass, .restitution = 0.5f});
         }
      }
   }

   {
      const glm::vec3 position{0.0f, -0.5f, 0.0f};                      // NOLINT
      const glm::vec3 dimensions{100.0f, 0.5f, 100.0f};                 // NOLINT
      const glm::vec3 colour{192 / 255.0f, 192 / 255.0f, 192 / 255.0f}; // NOLINT

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

   {
      const glm::vec3 position{50.5f, 0.0f, 0.0f};      // NOLINT
      const glm::vec3 dimensions{0.5f, 100.0f, 100.0f}; // NOLINT

      auto entity = m_registry.create();
      m_registry.emplace<collision::component::box_collider>(
         entity, collision::component::box_collider{.center = position, .half_size = dimensions});
   }

   add_invisible_wall({50.5f, 0.0f, 0.0f}, {0.5f, 100.0f, 100.0f});  // NOLINT
   add_invisible_wall({-50.5f, 0.0f, 0.0f}, {0.5f, 100.0f, 100.0f}); // NOLINT
   add_invisible_wall({0.0, 0.0f, -10.5f}, {100.0f, 100.0f, 0.5f});  // NOLINT
   add_invisible_wall({0.0, 0.0f, 10.5f}, {100.0f, 100.0f, 0.5f});   // NOLINT

   util::log_info(m_logger, "particle count = {}", x_count * y_count * z_count);
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

      {
         const auto old = start_time;
         start_time = chrono::steady_clock::now();
         delta_time = start_time - old;
      }
   }

   m_render_system.wait();
}

void simulation::update()
{
   compute_density(m_particles, m_settings);
   compute_normals(m_particles, m_settings);
   compute_forces(m_particles, m_settings);
   integrate(m_particles);
   resolve_collisions(m_particles);

   m_sph_system.update(m_settings.time_step);
   m_collision_system.update(m_settings.time_step);
}
void simulation::render()
{
   const auto image_index = m_render_system.begin_frame();

   m_camera.update(image_index.value(), compute_matrices(m_render_system));

   m_render_passes[0].record_render_calls([&](vk::CommandBuffer buffer) {
      auto& pipeline = check_err(m_pipelines.lookup(m_main_pipeline_key)).value();

      buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.value());

      buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0,
                                {m_camera.lookup_set(image_index.value())}, {});

      auto view = m_registry.view<component::render, component::transform>();

      buffer.bindVertexBuffers(0, {m_box.m_vertex_buffer->value()}, {vk::DeviceSize{0}});
      buffer.bindIndexBuffer(m_box.m_index_buffer->value(), 0, vk::IndexType::eUint32);

      for (auto entity : view | vi::filter([&](auto e) {
                            return view.get<component::render>(e).p_mesh == &m_box;
                         }))
      {
         const auto& render = view.get<component::render>(entity);
         const auto& transform = view.get<component::transform>(entity);

         mesh_data md{.model = transform.scale * transform.translate,
                      .colour = render.colour}; // NOLINT

         buffer.pushConstants(pipeline.layout(),
                              pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
                              sizeof(mesh_data) * 1, &md);

         buffer.drawIndexed(m_box.m_index_buffer.index_count(), 1, 0, 0, 0);
      }

      buffer.bindVertexBuffers(0, {m_sphere.m_vertex_buffer->value()}, {vk::DeviceSize{0}});
      buffer.bindIndexBuffer(m_sphere.m_index_buffer->value(), 0, vk::IndexType::eUint32);

      for (const auto& particle : m_particles)
      {
         const auto scale_factor = m_settings.scale_factor;
         const auto scale_vector = glm::vec3{scale_factor, scale_factor, scale_factor};
         const auto trans = glm::translate(glm::mat4{1}, particle.position);
         mesh_data md{.model = glm::scale(trans, scale_vector),
                      .colour = {65 / 255.0f, 105 / 255.0f, 225 / 255.0f}}; // NOLINT

         buffer.pushConstants(pipeline.layout(),
                              pipeline.get_push_constant_ranges("mesh_data").stageFlags, 0,
                              sizeof(mesh_data) * 1, &md);

         buffer.drawIndexed(m_sphere.m_index_buffer.index_count(), 1, 0, 0, 0);
      }
   });

   m_render_system.render(m_render_passes);
   m_render_system.end_frame();
}

void simulation::integrate(std::span<particle> particles)
{
   std::for_each(std::execution::par, std::begin(particles), std::end(particles), [&](auto& i) {
      i.velocity += m_settings.time_step * i.force / i.density;
      i.position += m_settings.time_step * i.velocity;
   });
}

auto get_closest_point(const collision::sphere& sphere,
                       const collision::component::box_collider& box) -> glm::vec3
{
   const auto x = std::clamp(sphere.center.x, box.center.x - box.half_size.x, // NOLINT
                             box.center.x + box.half_size.x);                 // NOLINT
   const auto y = std::clamp(sphere.center.y, box.center.y - box.half_size.y, // NOLINT
                             box.center.y + box.half_size.y);                 // NOLINT
   const auto z = std::clamp(sphere.center.z, box.center.z - box.half_size.z, // NOLINT
                             box.center.z + box.half_size.z);                 // NOLINT

   return {x, y, z};
}

auto get_distance(const collision::sphere& sphere, const collision::component::box_collider& box)
   -> float
{
   return glm::length(get_closest_point(sphere, box) - sphere.center);
}

void simulation::resolve_collisions(std::span<particle> particles)
{
   std::for_each(
      std::execution::par, std::begin(particles), std::end(particles), [&](particle& particle) {
         const float t0 = -m_settings.time_step;
         const float t1 = 0.0f;

         const auto p0 = particle.position + t0 * particle.velocity;
         const auto p1 = particle.position + t1 * particle.velocity;

         collision::sphere sphere_t0{p0, m_settings.water_radius};
         collision::sphere sphere_t1{p1, m_settings.water_radius};

         auto view = m_registry.view<collision::component::box_collider>();

         for (auto entity : view)
         {
            const auto& collider = view.get<collision::component::box_collider>(entity);

            const auto distance0 = get_distance(sphere_t0, collider);
            const auto distance = get_distance(sphere_t1, collider);

            if (distance - m_settings.water_radius < collision::epsilon) // Handle collision
            {
               glm::vec3 p;
               if (distance0 > distance)
               {
                  const auto collision_point = get_closest_point(sphere_t0, collider);
                  const auto normal = glm::normalize(sphere_t0.center - collision_point);

                  p = sphere_t0.center + normal * (distance0 - m_settings.water_radius);
               }
               else
               {
                  const auto collision_point = get_closest_point(sphere_t1, collider);
                  const auto normal = glm::normalize(sphere_t1.center - collision_point);

                  p = sphere_t0.center + normal * (distance - m_settings.water_radius);
               }

               const auto collision_point =
                  get_closest_point({p, m_settings.water_radius}, collider);
               const auto normal = glm::normalize(p - collision_point);
               const auto closing_velocity = glm::dot(normal, particle.velocity);

               if (closing_velocity <= 0)
               {
                  auto update_velocity = -closing_velocity * particle.restitution;

                  auto acceleration = particle.force / particle.density;
                  auto acc_vel = glm::dot(acceleration, normal) * m_settings.time_step;

                  if (acc_vel < 0)
                  {
                     update_velocity += particle.restitution * acc_vel;

                     if (update_velocity < 0)
                     {
                        update_velocity = 0.0f;
                     }
                  }

                  const float delta_velocity = update_velocity - closing_velocity;
                  const float inverse_mass = 1 / particle.mass;
                  const auto impulse = (delta_velocity / inverse_mass) * normal;

                  particle.velocity += impulse * inverse_mass;
                  particle.position = p;
               }
            }
         }
      });
}

auto simulation::compare_distance(float d0, float d1) -> bool
{
   if (d0 < m_settings.water_radius && d1 < m_settings.water_radius)
   {
      return true;
   }

   return false;
}

void simulation::add_invisible_wall(const glm::vec3& position, const glm::vec3& dimensions)
{
   auto entity = m_registry.create();
   m_registry.emplace<collision::component::box_collider>(
      entity, collision::component::box_collider{.center = position, .half_size = dimensions});
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

   util::dynamic_array<vk::Viewport> pipeline_viewports;
   pipeline_viewports.emplace_back(m_render_system.viewport());

   util::dynamic_array<vk::Rect2D> pipeline_scissors;
   pipeline_scissors.emplace_back(m_render_system.scissor());

   util::dynamic_array<pipeline_shader_data> pipeline_shader_data;
   pipeline_shader_data.push_back(vertex_shader_data);
   pipeline_shader_data.push_back(fragment_shader_data);

   auto info = check_err(m_pipelines.insert({.device = m_render_system.device(),
                                             .render_pass = m_render_passes[0],
                                             .p_logger = m_logger,
                                             .bindings = m_render_system.vertex_bindings(),
                                             .attributes = m_render_system.vertex_attributes(),
                                             .viewports = pipeline_viewports,
                                             .scissors = pipeline_scissors,
                                             .shader_infos = pipeline_shader_data}));

   return info.key();
}
auto simulation::setup_camera(pipeline_index_t index) -> camera
{
   auto pipeline_info = check_err(m_pipelines.lookup(index));

   return create_camera(m_render_system, pipeline_info.value(), m_logger);
}
auto simulation::compute_matrices(const render_system& system) -> camera::matrices
{
   auto dimensions = system.scissor().extent;

   camera::matrices matrices{};
   matrices.projection = glm::perspective(
      glm::radians(90.0F), dimensions.width / (float)dimensions.height, 0.1F, 1000.0F); // NOLINT
   matrices.view =
      glm::lookAt(glm::vec3(20.0f, 20.0f, 70.0f), glm::vec3(0.0f, 5.0f, -10.0f), // NOLINT
                  glm::vec3(0.0F, 1.0F, 0.0F));
   matrices.projection[1][1] *= -1;

   return matrices;
}
