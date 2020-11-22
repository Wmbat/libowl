#pragma once

#include <glm/glm.hpp>

namespace ecs::component
{
   struct transform
   {
      glm::vec3 position;
      glm::vec3 scale;
   };
} // namespace ecs::component
