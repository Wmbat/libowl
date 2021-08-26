#ifndef SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP
#define SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP

#include <sph-simulation/components.hpp>
#include <sph-simulation/physics/components.hpp>

#include <glm/ext/vector_float3.hpp>

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

      std::array<physics::component::rigid_body*, 2> bodies;
      std::array<render::component::transform*, 2> transforms;
   };
} // namespace physics

#endif // SPH_SIMULATION_PHYSICS_COLLISION_CONTACT_HPP
