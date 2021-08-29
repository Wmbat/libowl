#include <sph-simulation/sph/collision/detection.hpp>

namespace sph
{
   auto detect_particle_and_plane_collision(const particle_view& particles,
                                            const plane_view& planes) -> std::vector<contact>
   {
      std::vector<contact> contacts;

      for (auto particle_entity : particles)
      {
         auto& particle = particles.get<sph::particle>(particle_entity);
         auto& particle_transform = particles.get<transform>(particle_entity);
         const auto& sphere_collider = particles.get<physics::sphere_collider>(particle_entity);

         const auto sphere_position = sphere_collider.volume.center + particle_transform.position;

         for (auto plane_entity : planes)
         {
            const auto& plane_collider = planes.get<physics::plane_collider>(plane_entity);

            const auto plane_normal = plane_collider.volume.normal;

            const auto distance = glm::dot(plane_normal, sphere_position) -
               sphere_collider.volume.radius - plane_collider.volume.offset;

            if (distance < 0) // We have a collision
            {
               contacts.push_back({.point = sphere_position -
                                      plane_normal * (distance + sphere_collider.volume.radius),
                                   .normal = plane_normal,
                                   .penetration_depth = -distance,
                                   .friction = sphere_collider.friction,
                                   .restitution = sphere_collider.restitution,
                                   .p_particle = &particle,
                                   .p_particle_transform = &particle_transform,
                                   .p_rigid_body = nullptr,
                                   .p_rigid_body_transform = nullptr});
            }
         }
      }

      return contacts;
   }
} // namespace sph
