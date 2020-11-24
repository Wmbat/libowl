#pragma once

#include <water_simulation/core.hpp>

#include <util/containers/dynamic_array.hpp>

#include <entt/entt.hpp>

#include <glm/vec3.hpp>

namespace sph
{
   class grid
   {
      struct cell
      {
         util::dynamic_array<entt::entity> m_entities;
      };

   public:
      grid(float cell_size, const glm::vec3& center, const glm::vec3& dimensions);

   private:
      [[maybe_unused]] float m_cell_size;

      [[maybe_unused]] const glm::vec3 m_center;
      [[maybe_unused]] const glm::vec3 m_dimensions;

      [[maybe_unused]] util::dynamic_array<cell> m_cells;

      [[maybe_unused]] util::logger_ptr m_logger;
   };
} // namespace sph
