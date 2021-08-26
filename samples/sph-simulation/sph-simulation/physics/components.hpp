#ifndef SPH_SIMULATION_PHYSICS_COMPONENTS_HPP
#define SPH_SIMULATION_PHYSICS_COMPONENTS_HPP

#include <sph-simulation/physics/bounding_volume.hpp>

namespace physics::component
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
   
   struct rigid_body
   {
      glm::vec3 velocity{};
      glm::vec3 force{};

      float mass{1.0F};
      float density{0.0F};
   };
} // namespace physics::component

#endif // SPH_SIMULATION_PHYSICS_COMPONENTS_HPP
