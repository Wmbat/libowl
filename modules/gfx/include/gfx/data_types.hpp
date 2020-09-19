#pragma once

#include <util/containers/dynamic_array.hpp>

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
      glm::mat4 perspective;
      glm::mat4 view;
   };

   struct renderable_data
   {
      util::dynamic_array<vertex> vertices;
      util::dynamic_array<std::uint32_t> indices;
      glm::mat4 model;
   };
} // namespace gfx
