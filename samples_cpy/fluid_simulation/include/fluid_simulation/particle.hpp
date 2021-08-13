#pragma once

#include <glm/vec3.hpp>

struct particle
{
   glm::vec3 position{};
   glm::vec3 velocity{};
   glm::vec3 force{};
   glm::vec3 normal{};

   float mass{1.0F};
   float density{0.0F};
   float pressure{0.0F};
   float restitution{0.5f};
};
