#ifndef SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP
#define SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP

#include <glm/ext/vector_float3.hpp>
namespace physics
{
   struct contact
   {
      glm::vec3 point;
      glm::vec3 normal;

      float penetration_depth;
   };
} // namespace physics

#endif // SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP
