#ifndef SPH_SIMULATION_SPH_PARTICLE_HPP
#define SPH_SIMULATION_SPH_PARTICLE_HPP

#include <glm/ext/vector_float3.hpp>

namespace sph
{
   /**
    * @brief Data necessary to perform the SPH computations on a particle of fluid.
    */
   struct particle
   {
      glm::vec3 velocity{};
      glm::vec3 force{};
      glm::vec3 normal{};

      float radius{1.0f};
      float mass{1.0F};
      float density{0.0F};
      float pressure{0.0F};
   };
} // namespace sph

#endif // SPH_SIMULATION_SPH_PARTICLE_HPP
