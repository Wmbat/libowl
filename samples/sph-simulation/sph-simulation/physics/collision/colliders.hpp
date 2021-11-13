#ifndef SPH_SIMULATION_PHYSICS_COLLISION_COLLIDERS_HPP
#define SPH_SIMULATION_PHYSICS_COLLISION_COLLIDERS_HPP

#include <sph-simulation/physics/bounding_volume.hpp>

namespace physics
{
   struct sphere_collider
   {
      sphere_volume volume; 
      float friction;
      float restitution;
   };

   struct box_collider
   {
      box_volume volume;
      float friction;
      float restitution;
   };

   struct plane_collider
   {
      plane_volume volume;
      float friction;
      float restitution;
   };
} // namespace physics

#endif // SPH_SIMULATION_PHYSICS_COLLISION_COLLIDERS_HPP
