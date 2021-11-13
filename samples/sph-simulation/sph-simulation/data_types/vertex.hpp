/**
 * @file sph-simulation/data_types/vertex.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef SPH_SIMULATION_DATA_TYPES_VERTEX_HPP_
#define SPH_SIMULATION_DATA_TYPES_VERTEX_HPP_

// Third Party Libraries

#if defined(__GNUC__) && not defined(__clang__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wvolatile"
#elif defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-volatile"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#if defined(__GNUC__) && not defined(__clang__)
#   pragma GCC diagnostic pop
#elif defined(__clang__)
#   pragma clang diagnostic pop
#endif

// C++ Standard Library

#include <vector>

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

struct renderable_data
{
   std::vector<vertex> vertices;
   std::vector<std::uint32_t> indices;
   glm::mat4 model{};
};

namespace std
{
   template <>
   struct hash<vertex>
   {
      auto operator()(const vertex& vertex) const -> std::size_t
      {
         return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.colour) << 1)) >>
                 1);
      }
   };
} // namespace std

#endif // SPH_SIMULATION_DATA_TYPES_VERTEX_HPP_
