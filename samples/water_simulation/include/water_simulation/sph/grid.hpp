#pragma once

#include <water_simulation/core.hpp>

#include <util/containers/dynamic_array.hpp>

#include <entt/entt.hpp>

#include <glm/vec3.hpp>

#include <span>

namespace sph
{
   struct particle
   {
      glm::i64vec3 grid_position{};

      glm::vec3 position{};
      glm::vec3 velocity{};
      glm::vec3 force{};
      glm::vec3 normal{};

      float radius{1.0f};
      float mass{1.0F};
      float density{0.0F};
      float pressure{0.0F};
      float restitution{0.5f};
   };

   class grid
   {
   public:
      struct cell
      {
         glm::u64vec3 grid_pos{};
         glm::vec3 center{};

         util::dynamic_array<particle*> particles{};
      };

   public:
      grid() = default;
      grid(float cell_size, const glm::vec3& dimensions, vml::non_null<util::logger*> p_logger);

      /**
       * @brief Update the position of particles within the `grid`
       *
       * @param particles  The current state of the particles
       */
      void update_layout(std::span<particle> particles);

      /**
       * @brief Give access to all cells within the grid
       *
       * @return a view into all the cells within the grid
       */
      [[nodiscard]] auto cells() -> std::span<cell>;

      /**
       * @brief find and retrieve all entities from neighbouring cells from the passed cell
       *
       * @return A list of neighbouring entities.
       */
      auto lookup_neighbours(const glm::i64vec3& grid_pos) -> util::dynamic_array<particle*>;

   private:
      float m_cell_size{};
      glm::vec3 m_dimensions{};

      glm::i64vec3 m_cell_count{};

      util::dynamic_array<cell> m_cells{};

      util::logger_wrapper m_logger;
   };
} // namespace sph
