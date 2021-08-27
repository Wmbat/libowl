#pragma once

#include <sph-simulation/render/renderable.hpp>

#include <glm/glm.hpp>

namespace render::component
{
   struct render
   {
      renderable* p_mesh;
      glm::vec3 colour;
   };
} // namespace render::component
