#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace collision
{
   constexpr float epsilon = 0.0001f;

   struct plane
   {
      glm::vec3 normal;
      float offset;
   };

   struct box
   {
      glm::vec3 center;
      glm::vec3 half_size;
   };

   struct sphere
   {
      glm::vec3 center;
      float radius;
   };
} // namespace collision
