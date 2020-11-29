#include <water_simulation/sph/system.hpp>

#include <water_simulation/physics/kernel.hpp>

#include <range/v3/view/transform.hpp>

#include <glm/ext/quaternion_geometric.hpp>

#include <execution>

namespace vi = ranges::views;

namespace sph
{
   system::system(create_info&& info) :
      mp_registry{info.p_registry.get()}, m_grid{info.system_settings.kernel_radius(),
                                                 info.dimensions, info.p_logger},
      m_settings{info.system_settings},
      m_kernel_radius{m_settings.kernel_radius()}, m_logger{info.p_logger}
   {}

   void system::update(duration<float> time_step)
   {
      m_grid.update_layout(vml::make_not_null(mp_registry));

      compute_density_pressure();
      compute_normals();
      compute_forces();
      integrate(time_step);
   }

   void system::compute_density_pressure()
   {
      auto access_particle = [&](entt::entity e) -> component::particle& {
         return mp_registry->get<component::particle>(e);
      };

      std::for_each(std::execution::par, std::begin(m_grid.cells()), std::end(m_grid.cells()),
                    [&](auto& cell) {
                       auto neighbours = m_grid.lookup_neighbours(cell);

                       for (auto& particle_i : cell.entities | vi::transform(access_particle))
                       {
                          float density = 0.0F;

                          for (const auto& particle_j : neighbours | vi::transform(access_particle))
                          {
                             const auto r_ij = particle_j.position - particle_i.position;
                             const auto r = glm::length(r_ij);

                             if (r <= m_kernel_radius)
                             {
                                density += kernel::poly6(m_kernel_radius, r);
                             }
                          }

                          float density_ratio = particle_i.density / m_settings.rest_density;

                          particle_i.density =
                             density * kernel::poly6_constant(m_kernel_radius) * particle_i.mass;
                          particle_i.pressure =
                             density_ratio < 1.0f ? 0 : std::pow(density_ratio, 7.0f) - 1.0f;
                       }
                    });
   }
   void system::compute_normals()
   {
      auto access_particle = [&](entt::entity e) -> component::particle& {
         return mp_registry->get<component::particle>(e);
      };

      std::for_each(std::execution::par, std::begin(m_grid.cells()), std::end(m_grid.cells()),
                    [&](auto& cell) {
                       const auto neighbours = m_grid.lookup_neighbours(cell);

                       for (auto& particle_i : cell.entities | vi::transform(access_particle))
                       {
                          glm::vec3 normal{0.0f, 0.0f, 0.0f};

                          for (const auto& particle_j : neighbours | vi::transform(access_particle))
                          {
                             const auto r_ij = particle_j.position - particle_i.position;
                             const auto r = glm::length(r_ij);

                             if (r <= m_kernel_radius)
                             {
                                normal += kernel::poly6_grad(r_ij, m_kernel_radius, r) /
                                   particle_j.density;
                             }
                          }

                          particle_i.normal = normal *
                             (particle_i.radius * m_kernel_radius *
                              kernel::poly6_grad_constant(m_kernel_radius));
                       }
                    });
   }
   void system::compute_forces()
   {
      auto access_particle = [&](entt::entity e) -> component::particle& {
         return mp_registry->get<component::particle>(e);
      };

      const glm::vec3 gravity_vector{0.0f, gravity * gravity_multiplier, 0.0f};

      std::for_each(
         std::execution::par, std::begin(m_grid.cells()), std::end(m_grid.cells()),
         [&](auto& cell) {
            const auto neighbours = m_grid.lookup_neighbours(cell);

            for (auto& particle_i : cell.entities | vi::transform(access_particle))
            {
               glm::vec3 pressure_force{0.0f, 0.0f, 0.0f};
               glm::vec3 viscosity_force{0.0f, 0.0f, 0.0f};
               glm::vec3 cohesion_force{0.0f, 0.0f, 0.0f};
               glm::vec3 curvature_force{0.0f, 0.0f, 0.0f};
               glm::vec3 gravity_force{0.0f, 0.0f, 0.0f};

               for (const auto& particle_j : neighbours | vi::transform(access_particle))
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

                     if (r < m_kernel_radius)
                     {
                        pressure_force -= ((particle_i.pressure + particle_j.pressure) /
                                           (2.0f * particle_j.density)) *
                           kernel::spiky_grad(r_ij, m_kernel_radius, r);

                        if (particle_j.density > 0.00001f) // NOLINT
                        {
                           viscosity_force -=
                              ((particle_j.velocity - particle_i.velocity) / particle_j.density) *
                              kernel::viscosity(m_kernel_radius, r);
                        }

                        const float correction_factor = 2.0f * m_settings.rest_density /
                           (particle_i.density + particle_j.density);

                        cohesion_force +=
                           correction_factor * (r_ij / r) * kernel::cohesion(m_kernel_radius, r);
                        curvature_force +=
                           correction_factor * (particle_i.normal - particle_j.normal);
                     }
                  }
               }

               gravity_force += gravity_vector * particle_i.density;

               pressure_force *= kernel::spiky_grad_constant(m_kernel_radius) * particle_i.mass;
               viscosity_force *= m_settings.viscosity_constant *
                  kernel::viscosity_constant(m_kernel_radius) * particle_i.mass;
               cohesion_force *= -m_settings.surface_tension_coefficient *
                  kernel::cohesion_constant(m_kernel_radius) * square(particle_i.mass);
               curvature_force *= -m_settings.surface_tension_coefficient;

               particle_i.force = viscosity_force + pressure_force + cohesion_force +
                  curvature_force + gravity_force;
            }
         });
   }
   void system::integrate(duration<float> time_step)
   {
      auto access_particle = [&](entt::entity e) -> component::particle& {
         return mp_registry->get<component::particle>(e);
      };

      std::for_each(std::execution::par, std::begin(m_grid.cells()), std::end(m_grid.cells()),
                    [&](grid::cell& c) {
                       for (auto& i : c.entities | vi::transform(access_particle))
                       {
                          i.velocity += time_step.count() * i.force / i.density;
                          i.position += time_step.count() * i.velocity;
                       }
                    });
   }
} // namespace sph
