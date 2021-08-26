#ifndef SPH_SIMULATION_SPH_COMPONENTS_HPP
#define SPH_SIMULATION_SPH_COMPONENTS_HPP

#include <glm/vec3.hpp>

namespace sph::component
{
   struct particle
   {
      glm::vec3 velocity{};
      glm::vec3 force{};
      glm::vec3 normal{};

      float radius{1.0f};
      float mass{1.0F};
      float density{0.0F};
      float pressure{0.0F};
      float restitution{0.5f};
   };

   struct particle_data
   {
      glm::vec3 normal{};

      float radius{1.0f};
      float pressure{0.0F};
   };
} // namespace sph::component

#endif // SPH_SIMULATION_SPH_COMPONENTS_HPP
