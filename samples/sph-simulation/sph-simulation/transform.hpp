#ifndef SPH_SIMULATION_TRANSFORM_HPP
#define SPH_SIMULATION_TRANSFORM_HPP

#include <glm/ext/vector_float3.hpp>

struct transform
{
   glm::vec3 position;
   glm::vec3 rotation;
   glm::vec3 scale;
};

#endif // SPH_SIMULATION_TRANSFORM_HPP
