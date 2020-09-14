#pragma once

#include <glm/glm.hpp>

namespace gfx
{
   struct vertex
   {
      glm::vec3 position;
      glm::vec3 colour;
   };

   struct camera_matrices
   {
      glm::mat4 model;
      glm::mat4 perspective;
      glm::mat4 view;
   };
} // namespace gfx
