#pragma once

#include <sph-simulation/render/renderable.hpp>

#include <glm/glm.hpp>

namespace render::component
{
   struct transform
   {
      glm::vec3 position;
      glm::vec3 rotation;
      glm::vec3 scale;
   };

   struct render
   {
      renderable* p_mesh;
      glm::vec3 colour;
   };
} // namespace render::component
