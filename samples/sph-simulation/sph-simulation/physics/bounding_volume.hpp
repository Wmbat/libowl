#ifndef SPH_SIMULATION_PHYSICS_BOUNDING_VOLUME_HPP
#define SPH_SIMULATION_PHYSICS_BOUNDING_VOLUME_HPP

#include <glm/vec3.hpp>

namespace physics
{
   struct sphere_volume
   {
      glm::vec3 center;
      float radius;
   };

   struct box_volume
   {
      glm::vec3 center;
      glm::vec3 half_dimensions;
   };

   struct plane_volume
   {
      glm::vec3 normal;
      float offset;
   };
} // namespace physics

#endif // SPH_SIMULATION_PHYSICS_BOUNDING_VOLUME_HPP
