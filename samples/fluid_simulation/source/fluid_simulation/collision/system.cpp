#include <fluid_simulation/collision/system.hpp>

#include <fluid_simulation/sph/grid.hpp>

#include <glm/ext/quaternion_geometric.hpp>

#include <entt/entt.hpp>

#include <execution>

namespace collision
{
   auto get_closest_point(const component::sphere_collider& sphere,
                          const component::box_collider& box) -> glm::vec3
   {
      const auto x = std::clamp(sphere.center.x, box.center.x - box.half_size.x, // NOLINT
                                box.center.x + box.half_size.x);                 // NOLINT
      const auto y = std::clamp(sphere.center.y, box.center.y - box.half_size.y, // NOLINT
                                box.center.y + box.half_size.y);                 // NOLINT
      const auto z = std::clamp(sphere.center.z, box.center.z - box.half_size.z, // NOLINT
                                box.center.z + box.half_size.z);                 // NOLINT

      return {x, y, z};
   }

   auto get_distance(const component::sphere_collider& sphere,
                     const collision::component::box_collider& box) -> float
   {
      return glm::length(get_closest_point(sphere, box) - sphere.center);
   }

   system::system(create_info&& info) :
      mp_registry{info.p_registry.get()}, mp_system{info.p_sph_system.get()}
   {}

   void system::update(duration<float> time_step) { resolve_particle_to_box_collision(time_step); }

   void system::resolve_particle_to_box_collision(duration<float> time_step)
   {
      /*
      auto view = mp_registry->view<sph::component::particle>();
      */

      std::for_each(
         std::begin(mp_system->particles()), std::end(mp_system->particles()), [&](auto& particle) {
            const float t0 = -time_step.count();
            const float t1 = 0.0f;

            const auto p0 = particle.position + t0 * particle.velocity;
            const auto p1 = particle.position + t1 * particle.velocity;

            component::sphere_collider sphere_t0{p0, particle.radius};
            component::sphere_collider sphere_t1{p1, particle.radius};

            auto view = mp_registry->view<collision::component::box_collider>();
            for (auto entity : view)
            {
               const auto& collider = view.get<collision::component::box_collider>(entity);

               const auto distance0 = get_distance(sphere_t0, collider);
               const auto distance = get_distance(sphere_t1, collider);

               if (distance - particle.radius < collision::epsilon) // Handle collision
               {
                  glm::vec3 p;
                  if (distance0 > distance)
                  {
                     const auto collision_point = get_closest_point(sphere_t0, collider);
                     const auto normal = glm::normalize(sphere_t0.center - collision_point);

                     p = sphere_t0.center + normal * (distance0 - particle.radius);
                  }
                  else
                  {
                     const auto collision_point = get_closest_point(sphere_t1, collider);
                     const auto normal = glm::normalize(sphere_t1.center - collision_point);

                     p = sphere_t0.center + normal * (distance - particle.radius);
                  }

                  const auto collision_point = get_closest_point({p, particle.radius}, collider);
                  const auto normal = glm::normalize(p - collision_point);
                  const auto closing_velocity = glm::dot(normal, particle.velocity);

                  if (closing_velocity <= 0)
                  {
                     auto update_velocity = -closing_velocity * particle.restitution;

                     auto acceleration = particle.force / particle.density;
                     auto acc_vel = glm::dot(acceleration, normal) * time_step.count();

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
} // namespace collision
