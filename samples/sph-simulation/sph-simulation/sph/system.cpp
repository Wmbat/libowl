#include <sph-simulation/sph/system.hpp>

#include <sph-simulation/components.hpp>
#include <sph-simulation/physics/kernel.hpp>
#include <sph-simulation/sph/components.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtx/norm.hpp>

#include <execution>

namespace sph
{
   void compute_density_pressure(particle_view& particles, float m_kernel_radius,
                                 float rest_density)
   {
      parallel_for(particles, [&](const entt::entity& entity_i) {
         const auto& transform_i = particles.get<render::component::transform>(entity_i);
         auto& rigid_body_i = particles.get<physics::component::rigid_body>(entity_i);
         auto& particle_i = particles.get<component::particle_data>(entity_i);

         float density = 0.0f;

         for (auto& entity_j : particles)
         {
            const auto& transform_j = particles.get<render::component::transform>(entity_j);
            const auto& rigid_body_j = particles.get<physics::component::rigid_body>(entity_j);

            const auto r_ij = transform_i.position - transform_j.position;
            const auto r2 = glm::length2(r_ij);

            if (r2 <= square(m_kernel_radius))
            {
               density += rigid_body_j.mass * kernel::poly6(m_kernel_radius, r2);
            }
         }

         rigid_body_i.density = density * kernel::poly6_constant(m_kernel_radius);

         float ratio = rigid_body_i.density / rest_density;
         particle_i.pressure = ratio < 1.0f ? 0.0f : std::pow(ratio, 7.0f) - 1.0f;
      });
   }

   void compute_normals(particle_view& particles, float kernel_radius)
   {
      parallel_for(particles, [&](const entt::entity& entity_i) {
         const auto& i_transform = particles.get<render::component::transform>(entity_i);
         auto& i_particle = particles.get<component::particle_data>(entity_i);

         glm::vec3 normal{0.0f, 0.0f, 0.0f};

         for (auto& entity_j : particles)
         {
            const auto& j_transform = particles.get<render::component::transform>(entity_j);
            const auto& j_rigid_body = particles.get<physics::component::rigid_body>(entity_j);

            const auto r_ij = i_transform.position - j_transform.position;
            const auto r2 = glm::length2(r_ij);
            const auto h2 = square(kernel_radius);

            if (r2 <= h2)
            {
               normal +=
                  (j_rigid_body.mass / j_rigid_body.density) * kernel::poly6_grad(r_ij, h2, r2);
            }
         }

         i_particle.normal = normal *
            (i_particle.radius * kernel_radius * kernel::poly6_grad_constant(kernel_radius));
      });
   }

   void compute_forces(particle_view& view, float kernel_radius, float rest_density,
                       float viscosity, float surface_tension, float gravity_mult)
   {
      const glm::vec3 gravity_vector{0.0f, gravity * gravity_mult, 0.0f};

      parallel_for(view, [&](const entt::entity& entity_i) {
         const auto& transform_i = view.get<render::component::transform>(entity_i);
         auto& rigid_body_i = view.get<physics::component::rigid_body>(entity_i);
         auto& particle_i = view.get<component::particle_data>(entity_i);

         glm::vec3 pressure_force{0.0f, 0.0f, 0.0f};
         glm::vec3 viscosity_force{0.0f, 0.0f, 0.0f};
         glm::vec3 cohesion_force{0.0f, 0.0f, 0.0f};
         glm::vec3 curvature_force{0.0f, 0.0f, 0.0f};
         glm::vec3 gravity_force{0.0f, 0.0f, 0.0f};

         for (auto& entity_j : view)
         {
            const auto& transform_j = view.get<render::component::transform>(entity_j);
            const auto& rigid_body_j = view.get<physics::component::rigid_body>(entity_j);
            const auto& particle_j = view.get<component::particle_data>(entity_j);

            if (entity_i != entity_j)
            {
               glm::vec3 r_ij = transform_i.position - transform_j.position;
               if (r_ij.x == 0.0f && r_ij.y == 0.0f) // NOLINT
               {
                  r_ij.x += 0.0001f; // NOLINT
                  r_ij.y += 0.0001f; // NOLINT
               }

               const auto r = glm::length(r_ij);

               if (r < kernel_radius)
               {
                  pressure_force += glm::normalize(r_ij) *
                     (rigid_body_j.mass * (particle_i.pressure + particle_j.pressure) /
                      (2.0f * rigid_body_j.density) * kernel::spiky(kernel_radius, r));

                  viscosity_force += rigid_body_j.mass *
                     ((rigid_body_j.velocity - rigid_body_i.velocity) / rigid_body_j.density) *
                     kernel::viscosity(kernel_radius, r);

                  const float correction_factor =
                     (2.0f * rest_density) / (rigid_body_i.density + rigid_body_j.density);

                  cohesion_force += ((transform_i.position - transform_j.position) / r) *
                     (kernel::cohesion(kernel_radius, r) * correction_factor);
                  curvature_force += correction_factor * (particle_i.normal - particle_j.normal);
               }
            }
         }

         gravity_force += gravity_vector * rigid_body_i.density;
         pressure_force *= kernel::spiky_constant(kernel_radius);
         viscosity_force *= viscosity * kernel::viscosity_constant(kernel_radius);

         cohesion_force *=
            -surface_tension * kernel::cohesion_constant(kernel_radius) * square(rigid_body_i.mass);
         curvature_force *= -surface_tension;

         rigid_body_i.force =
            viscosity_force + pressure_force + cohesion_force + curvature_force + gravity_force;
      });
   }

   void integrate(particle_view& particles, duration<float> time_step)
   {
      parallel_for(particles, [&](const entt::entity& entity) {
         auto& transform = particles.get<render::component::transform>(entity);
         auto& rigid_body = particles.get<physics::component::rigid_body>(entity);

         rigid_body.velocity += time_step.count() * rigid_body.force / rigid_body.density;
         transform.position += time_step.count() * rigid_body.velocity;
      });
   }

   void update(particle_view& particles, const sim_variables& variables, duration<float> time_step)
   {
      const auto kernel_radius = compute_kernel_radius(variables);

      compute_density_pressure(particles, kernel_radius, variables.rest_density);
      compute_normals(particles, kernel_radius);
      compute_forces(particles, kernel_radius, variables.rest_density, variables.viscosity_constant,
                     variables.surface_tension_coefficient, variables.gravity_multiplier);
      integrate(particles, time_step);
   }
} // namespace sph
