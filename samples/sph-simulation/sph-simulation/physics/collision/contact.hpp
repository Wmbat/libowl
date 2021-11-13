#ifndef SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP
#define SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP

#include <sph-simulation/physics/rigid_body.hpp>
#include <sph-simulation/transform.hpp>

#include <array>

namespace physics
{
   struct contact
   {
      glm::vec3 point;
      glm::vec3 normal;

      float penetration_depth;
      float friction;
      float restitution;

      std::array<physics::rigid_body*, 2> bodies;
      std::array<::transform*, 2> transforms;
   };
} // namespace physics

#endif // SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP
