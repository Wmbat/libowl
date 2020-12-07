#include <water_simulation/sph/system.hpp>

#include <water_simulation/physics/kernel.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtx/norm.hpp>

#include <execution>

namespace vi = ranges::views;

namespace sph
{
   system::system(create_info&& info) :
      mp_registry{info.p_registry.get()}, m_logger{info.p_logger}, m_settings{info.system_settings},
      m_kernel_radius{m_settings.kernel_radius()}, m_grid{m_settings.kernel_radius(),
                                                          info.dimensions, info.p_logger}
   {}

   void system::emit(particle&& particle) { m_particles.emplace_back(particle); }

   auto system::particles() -> std::span<particle> { return m_particles; }

   void system::update(duration<float> time_step)
   {
      m_grid.update_layout(m_particles);

      compute_density_pressure();
      compute_normals();
      compute_forces();
      integrate(time_step);
   }

   void system::compute_density_pressure()
   {
      std::for_each(std::execution::par, std::begin(m_grid.cells()), std::end(m_grid.cells()),
                    [&](auto& cell) {
                       auto neighbours = m_grid.lookup_neighbours(cell.grid_pos);

                       std::for_each(std::execution::par, std::begin(cell.particles),
                                     std::end(cell.particles), [&](auto* particle_i) {
                                        float density = 0.0F;

                                        for (const auto* particle_j : neighbours)
                                        {
                                           const auto r_ij =
                                              particle_i->position - particle_j->position;
                                           const auto r2 = glm::length2(r_ij);

                                           if (r2 <= square(m_kernel_radius))
                                           {
                                              density += particle_j->mass *
                                                 kernel::poly6(m_kernel_radius, r2) *
                                                 kernel::poly6_constant(m_kernel_radius);
                                           }
                                        }

                                        particle_i->density = density;

                                        float ratio = particle_i->density / m_settings.rest_density;
                                        particle_i->pressure =
                                           ratio < 1.0f ? 0.0f : std::pow(ratio, 7.0f) - 1.0f;
                                     });
                    });
   }
   void system::compute_normals()
   {
      std::for_each(std::execution::par, std::begin(m_grid.cells()), std::end(m_grid.cells()),
                    [&](auto& cell) {
                       const auto neighbours = m_grid.lookup_neighbours(cell.grid_pos);

                       std::for_each(std::execution::par, std::begin(cell.particles),
                                     std::end(cell.particles), [&](auto* particle_i) {
                                        glm::vec3 normal{0.0f, 0.0f, 0.0f};

                                        for (const auto* particle_j : neighbours)
                                        {
                                           const auto r_ij =
                                              particle_i->position - particle_j->position;
                                           const auto r2 = glm::length2(r_ij);
                                           const auto h2 = square(m_kernel_radius);

                                           if (r2 <= h2)
                                           {
                                              normal += (particle_j->mass / particle_j->density) *
                                                 kernel::poly6_grad(r_ij, h2, r2);
                                           }
                                        }

                                        particle_i->normal = normal *
                                           (particle_i->radius * m_kernel_radius *
                                            kernel::poly6_grad_constant(m_kernel_radius));
                                     });
                    });
   }

   void system::compute_forces()
   {
      const glm::vec3 gravity_vector{0.0f, gravity * m_settings.gravity_multiplier, 0.0f};

      std::for_each(
         std::execution::par, std::begin(m_grid.cells()), std::end(m_grid.cells()),
         [&](auto& cell) {
            const auto neighbours = m_grid.lookup_neighbours(cell.grid_pos);

            std::for_each(
               std::execution::par, std::begin(cell.particles), std::end(cell.particles),
               [&](auto* particle_i) {
                  glm::vec3 pressure_force{0.0f, 0.0f, 0.0f};
                  glm::vec3 viscosity_force{0.0f, 0.0f, 0.0f};
                  glm::vec3 cohesion_force{0.0f, 0.0f, 0.0f};
                  glm::vec3 curvature_force{0.0f, 0.0f, 0.0f};
                  glm::vec3 gravity_force{0.0f, 0.0f, 0.0f};

                  for (const auto* particle_j : neighbours)
                  {
                     if (particle_i != particle_j)
                     {
                        glm::vec3 r_ij = particle_i->position - particle_j->position;
                        if (r_ij.x == 0.0f && r_ij.y == 0.0f) // NOLINT
                        {
                           r_ij.x += 0.0001f; // NOLINT
                           r_ij.y += 0.0001f; // NOLINT
                        }

                        const auto r = glm::length(r_ij);

                        if (r < m_kernel_radius)
                        {
                           pressure_force += glm::normalize(r_ij) *
                              (particle_j->mass * (particle_i->pressure + particle_j->pressure) /
                               (2.0f * particle_j->density) *
                               kernel::spiky_constant(m_kernel_radius) *
                               kernel::spiky(m_kernel_radius, r));

                           viscosity_force += m_settings.viscosity_constant * particle_j->mass *
                              ((particle_j->velocity - particle_i->velocity) /
                               particle_j->density) *
                              kernel::viscosity_constant(m_kernel_radius) *
                              kernel::viscosity(m_kernel_radius, r);

                           const float correction_factor = (2.0f * m_settings.rest_density) /
                              (particle_i->density + particle_j->density);

                           cohesion_force += ((particle_i->position - particle_j->position) / r) *
                              (kernel::cohesion(m_kernel_radius, r) * correction_factor);
                           curvature_force +=
                              correction_factor * (particle_i->normal - particle_j->normal);
                        }
                     }
                  }

                  gravity_force += gravity_vector * particle_i->density;

                  cohesion_force *= -m_settings.surface_tension_coefficient *
                     kernel::cohesion_constant(m_kernel_radius) * square(particle_i->mass);
                  curvature_force *= -m_settings.surface_tension_coefficient;

                  particle_i->force = viscosity_force + pressure_force + cohesion_force +
                     curvature_force + gravity_force;
               });
         });
   }
   void system::integrate(duration<float> time_step)
   {
      std::for_each(std::execution::par, std::begin(m_particles), std::end(m_particles),
                    [&](auto& particle) {
                       particle.velocity += time_step.count() * particle.force / particle.density;
                       particle.position += time_step.count() * particle.velocity;
                    });
   }
} // namespace sph
