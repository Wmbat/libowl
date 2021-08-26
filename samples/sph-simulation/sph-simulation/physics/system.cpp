#include <sph-simulation/physics/system.hpp>

#include <sph-simulation/components.hpp>
#include <sph-simulation/physics/components.hpp>
#include <sph-simulation/sph/components.hpp>

#include <entt/entt.hpp>

namespace physics
{
   void update(entt::registry* p_registry, duration<float> time_step)
   {
      auto sphere_view =
         p_registry
            ->view<component::sphere_collider, sph::component::particle, render::component::transform>();
      auto box_view = p_registry->view<component::box_collider>();

      for (auto box_entity : box_view)
      {
         const auto& box_collider = box_view.get<component::box_collider>(box_entity);

         for (auto sphere_entity : sphere_view)
         {
            auto& transform = sphere_view.get<render::component::transform>(sphere_entity);
            auto& particle = sphere_view.get<sph::component::particle>(sphere_entity);
         }
      }
   }
} // namespace physics
