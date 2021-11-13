#pragma once

#include <sph-simulation/render/renderable.hpp>

#include <glm/glm.hpp>

namespace component
{
   struct mesh
   {
      const renderable* p_mesh;
      glm::vec3 colour;
   };
} // namespace component
