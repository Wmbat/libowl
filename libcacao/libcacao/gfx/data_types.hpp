#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>

namespace cacao
{
   struct vertex
   {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec3 colour;
   };

   inline auto operator==(const vertex& lhs, const vertex& rhs) -> bool
   {
      return lhs.position == rhs.position && lhs.normal == rhs.normal && lhs.colour == rhs.colour;
   }

   struct camera_matrices
   {
      glm::mat4 perspective;
      glm::mat4 view;
   };

   struct renderable_data
   {
      std::vector<vertex> vertices;
      std::vector<std::uint32_t> indices;
      glm::mat4 model{};
   };
} // namespace cacao

namespace std
{
   template <>
   struct hash<cacao::vertex>
   {
      auto operator()(const cacao::vertex& vertex) const -> std::size_t
      {
         return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.colour) << 1)) >>
                 1);
      }
   };
} // namespace std
