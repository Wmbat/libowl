#pragma once

#include <glm/vec3.hpp>

struct contact
{
   glm::vec3 position;
   glm::vec3 normal;
   float interpenetration;
};
