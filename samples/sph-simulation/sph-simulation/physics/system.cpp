#include <sph-simulation/physics/system.hpp>

#include <sph-simulation/components.hpp>
#include <sph-simulation/physics/collision/colliders.hpp>
#include <sph-simulation/physics/collision/contact.hpp>

#include <entt/entt.hpp>

#include <glm/gtx/rotate_vector.hpp>

namespace physics
{
   auto get_closest_point(const physics::sphere_volume& sphere, const physics::box_volume& box)
      -> glm::vec3
   {
      const auto x = std::clamp(sphere.center.x, box.center.x - box.half_dimensions.x, // NOLINT
                                box.center.x + box.half_dimensions.x);                 // NOLINT
      const auto y = std::clamp(sphere.center.y, box.center.y - box.half_dimensions.y, // NOLINT
                                box.center.y + box.half_dimensions.y);                 // NOLINT
      const auto z = std::clamp(sphere.center.z, box.center.z - box.half_dimensions.z, // NOLINT
                                box.center.z + box.half_dimensions.z);                 // NOLINT

      return {x, y, z};
   }

   auto rotate_vec3(const glm::vec3& target, const glm::vec3& rotation) -> glm::vec3
   {
      const auto rotated_x = glm::rotateX(target, rotation.x);    // NOLINT
      const auto rotated_y = glm::rotateY(rotated_x, rotation.y); // NOLINT
      return glm::rotateZ(rotated_y, rotation.z);                 // NOLINT
   }

   auto detect_sphere_and_plane_collision(const sphere_view& spheres, const plane_view& planes)
      -> std::vector<contact>
   {
      std::vector<contact> contacts;

      for (auto sphere_entity : spheres)
      {
         auto& sphere_transform = spheres.get<render::component::transform>(sphere_entity);
         const auto& sphere_collider = spheres.get<physics::sphere_collider>(sphere_entity);
         auto& sphere_rigid_boy = spheres.get<rigid_body>(sphere_entity);

         const auto sphere_position = sphere_collider.volume.center + sphere_transform.position;

         for (auto plane_entity : planes)
         {
            const auto& plane_collider = planes.get<physics::plane_collider>(plane_entity);

            const auto plane_normal = plane_collider.volume.normal;

            const auto distance = glm::dot(plane_normal, sphere_position) -
               sphere_collider.volume.radius - plane_collider.volume.offset;

            if (distance < 0) // We have a collision
            {
               contacts.push_back(
                  contact{.point = sphere_position -
                             plane_normal * (distance + sphere_collider.volume.radius),
                          .normal = plane_normal,
                          .penetration_depth = -distance,
                          .friction = sphere_collider.friction,
                          .restitution = sphere_collider.restitution,
                          .bodies = {&sphere_rigid_boy, nullptr},
                          .transforms = {&sphere_transform, nullptr}});
            }
         }
      }

      return contacts;
   }

   auto detect_collisions(const system_update_info& info)
   {
      return detect_sphere_and_plane_collision(info.spheres, info.planes);
   }

   void update(const system_update_info& info)
   {
      sph::solve(info.particles, info.variables, info.time_step);

      const auto contacts = detect_collisions(info);

      for (auto contact_data : contacts)
      {
         auto* rb1 = contact_data.bodies[0];
         auto* transform1 = contact_data.transforms[0];

         const auto closing_vel = glm::dot(contact_data.normal, rb1->velocity);

         if (closing_vel <= 0)
         {
            auto update_velocity = -closing_vel * contact_data.restitution;

            auto acceleration = rb1->force / rb1->density;
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
            const float inverse_mass = 1 / rb1->mass;
            const auto impulse = (delta_velocity / inverse_mass) * contact_data.normal;

            rb1->velocity += impulse * inverse_mass;
            transform1->position += contact_data.normal * contact_data.penetration_depth;
         }
      }
   }
} // namespace physics
