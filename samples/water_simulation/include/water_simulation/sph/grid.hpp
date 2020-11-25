#pragma once

#include <water_simulation/core.hpp>

#include <util/containers/dynamic_array.hpp>

#include <entt/entt.hpp>

#include <glm/vec3.hpp>

#include <span>

namespace sph
{
   class grid
   {
   public:
      struct cell
      {
         glm::u64vec3 grid_pos{};
         glm::vec3 center{};
         glm::vec3 dimensions{};

         util::dynamic_array<entt::entity> m_entities{};
      };

   public:
      grid() = default;
      grid(float cell_size, const glm::vec3& dimensions, util::logger_ptr logger);

      /**
       * @brief Give access to all cells within the grid
       *
       * @return a view into all the cells within the grid
       */
      [[nodiscard]] auto cells() -> std::span<cell>;

      /**
       * @brief find and retrieve all entities from neighbouring cells from the passed cell
       *
       * @param cell The cell to use a center of neighbouroud search.
       *
       * @return A list of neighbouring entities.
       */
      auto lookup_neighbours(const cell& cell) -> util::dynamic_array<entt::entity>;

   private:
      const float m_cell_size{};
      const glm::vec3 m_dimensions{};

      glm::u64vec3 m_cell_count{};

      util::dynamic_array<cell> m_cells{};

      util::logger_ptr m_logger;
   };
} // namespace sph
