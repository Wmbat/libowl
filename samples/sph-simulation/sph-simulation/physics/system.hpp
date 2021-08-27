#ifndef SPH_SIMULATION_PHYSICS_SYSTEM_HPP
#define SPH_SIMULATION_PHYSICS_SYSTEM_HPP

#include <sph-simulation/components.hpp>
#include <sph-simulation/core.hpp>
#include <sph-simulation/transform.hpp>

#include <sph-simulation/physics/collision/colliders.hpp>
#include <sph-simulation/physics/rigid_body.hpp>
#include <sph-simulation/physics/sph/solver.hpp>

#include <entt/entt.hpp>

#define SPHERE_COMPONENTS transform, physics::sphere_collider, physics::rigid_body
#define BOX_COMPONENTS transform, physics::box_collider, physics::rigid_body
#define PLANE_COMPONENTS physics::plane_collider

namespace physics
{
   using sphere_view = entt::view<entt::exclude_t<>, SPHERE_COMPONENTS>;
   using box_view = entt::view<entt::exclude_t<>, BOX_COMPONENTS>;
   using plane_view = entt::view<entt::exclude_t<>, PLANE_COMPONENTS>;

   struct system_update_info
   {
      sph::particle_view particles;
      sphere_view spheres;
      plane_view planes;
      box_view boxes;

      const sim_variables& variables;

      duration<float> time_step;
   };

   void update(const system_update_info& info);
} // namespace physics

#endif // SPH_SIMULATION_PHYSICS_SYSTEM_HPP
