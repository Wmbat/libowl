#include <sph-simulation/sph/system.hpp>

#include <sph-simulation/sph/collision/detection.hpp>
#include <sph-simulation/sph/solver.hpp>

namespace sph
{
   void update(const system_update_info &info)
   {
      solve(info.particles, info.variables, info.time_step);

      auto contacts = detect_particle_and_plane_collision(info.particles, info.planes);

      for (auto contact_data : contacts)
      {
         auto* particle = contact_data.p_particle;
         auto* particle_transform = contact_data.p_particle_transform;

         const auto closing_vel = glm::dot(contact_data.normal, particle->velocity);

         if (closing_vel <= 0)
         {
            auto update_velocity = -closing_vel * contact_data.restitution;

            auto acceleration = particle->force / particle->density;
            auto acc_vel = glm::dot(acceleration, contact_data.normal) * info.time_step.count();

            if (acc_vel < 0)
            {
               update_velocity += contact_data.restitution * acc_vel;

               if (update_velocity < 0)
               {
                  update_velocity = 0.0f;
               }
            }

            const float delta_velocity = update_velocity - closing_vel;
            const float inverse_mass = 1 / particle->mass;
            const auto impulse = (delta_velocity / inverse_mass) * contact_data.normal;

            particle->velocity += impulse * inverse_mass;
            particle_transform->position += contact_data.normal * contact_data.penetration_depth;
         }
      }
   }
} // namespace sph
