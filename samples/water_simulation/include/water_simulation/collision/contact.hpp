#pragma once

#include <water_simulation/particle.hpp>

#include <glm/vec3.hpp>

namespace collision
{
   struct contact
   {
      particle& particle_1;
      particle& particle_2;

      glm::vec3 normal;
      float penetration;
   };
} // namespace collision
