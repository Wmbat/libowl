#pragma once

#include <water_simulation/core.hpp>

#include <util/containers/dynamic_array.hpp>

#include <entt/entt.hpp>

#include <glm/vec3.hpp>

#include <span>

namespace sph
{
   /**
    * @brief encapsulate all components within the `sph` submodule
    */
   namespace component
   {
      /**
       * @brief Compenent used for the handling of particles affected by the `sph::system`
       */
      struct particle
      {
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
   } // namespace component

   class grid
   {
   public:
      struct cell
      {
         glm::u64vec3 grid_pos{};
         glm::vec3 center{};

         util::dynamic_array<entt::entity> entities{};
      };

   public:
      grid() = default;
      grid(float cell_size, const glm::vec3& dimensions, vml::non_null<util::logger*> p_logger);

      void update_layout(vml::non_null<entt::registry*> p_registry);

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
      float m_cell_size{};
      glm::vec3 m_dimensions{};

      glm::u64vec3 m_cell_count{};

      util::dynamic_array<cell> m_cells{};

      util::logger_wrapper m_logger;
   };
} // namespace sph
