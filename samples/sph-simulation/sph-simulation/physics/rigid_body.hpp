#ifndef SPH_SIMULATION_PHYSICS_COMPONENTS_HPP
#define SPH_SIMULATION_PHYSICS_COMPONENTS_HPP

#include <sph-simulation/physics/bounding_volume.hpp>

namespace physics
{   
   struct rigid_body
   {
      glm::vec3 velocity{};
      glm::vec3 force{};

      float mass{1.0F};
      float density{0.0F};
   };
} // namespace physics

#endif // SPH_SIMULATION_PHYSICS_COMPONENTS_HPP
