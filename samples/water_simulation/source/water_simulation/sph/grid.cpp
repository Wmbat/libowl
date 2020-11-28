#include <water_simulation/sph/grid.hpp>

#include <range/v3/view/iota.hpp>

#include <glm/vec2.hpp>

#include <cassert>

namespace vi = ranges::views;

namespace sph
{
   grid::grid(float cell_size, const glm::vec3& dimensions, vml::non_null<util::logger*> p_logger) :
      m_cell_size{cell_size}, m_dimensions{dimensions}, m_logger{p_logger}
   {
      ENSURE(cell_size != 0);

      m_cell_count = glm::u64vec3{
         static_cast<std::size_t>(std::ceil(m_dimensions.x * 2 / m_cell_size)),  // NOLINT
         static_cast<std::size_t>(std::ceil(m_dimensions.y * 2 / m_cell_size)),  // NOLINT
         static_cast<std::size_t>(std::ceil(m_dimensions.z * 2 / m_cell_size))}; // NOLINT

      m_cells.reserve(m_cell_count.x + m_cell_count.y + m_cell_count.z); // NOLINT

      for (std::size_t x : vi::iota(0u, m_cell_count.x)) // NOLINT
      {
         for (std::size_t y : vi::iota(0u, m_cell_count.y)) // NOLINT
         {
            for (std::size_t z : vi::iota(0u, m_cell_count.z)) // NOLINT
            {
               glm::vec3 center{cell_size / 2.0f + static_cast<float>(x) * cell_size,
                                cell_size / 2.0f + static_cast<float>(y) * cell_size,
                                cell_size / 2.0f + static_cast<float>(z) * cell_size};
               glm::vec3 dims{cell_size / 2.0f, cell_size / 2.0f, cell_size / 2.0f};

               m_cells.push_back({.grid_pos = {x, y, z}, .center = center, .dimensions = dims});
            }
         }
      }

      m_logger.info("Grid constructed with a total of {} cells distributed as such:\n\t"
                    "x-axis = {}\n\ty-axis = {}\n\tz-axis = {}",
                    m_cell_count.x * m_cell_count.y * m_cell_count.z, m_cell_count.x, // NOLINT
                    m_cell_count.y, m_cell_count.z);                                  // NOLINT
   }

   void grid::update_layout(vml::non_null<entt::registry*> p_registry)
   {
      auto view = p_registry->view<component::particle>();

      for (auto& cell : m_cells)
      {
         m_logger.info("{} particles in cell ({}, {}, {})", std::size(cell.m_entities),
                       cell.grid_pos.x, cell.grid_pos.y, cell.grid_pos.z);

         cell.m_entities.clear();
      }

      for (auto entity : view)
      {
         [[maybe_unused]] auto& particle = p_registry->get<component::particle>(entity);

         for (auto& cell : m_cells)
         {
            bool within_x = (particle.position.x <= cell.center.x + cell.dimensions.x) &&
               (particle.position.x > cell.center.x - cell.dimensions.x);
            bool within_y = (particle.position.y <= cell.center.y + cell.dimensions.y) &&
               (particle.position.y > cell.center.y - cell.dimensions.y);
            bool within_z = (particle.position.z <= cell.center.z + cell.dimensions.z) &&
               (particle.position.z > cell.center.z - cell.dimensions.z);

            if (within_x && within_y && within_z)
            {
               cell.m_entities.push_back(entity);
            }
         }
      }
   }

   auto grid::cells() -> std::span<cell> { return m_cells; }

   auto grid::lookup_neighbours(const cell& cell) -> util::dynamic_array<entt::entity>
   {
      const auto& grip_pos = cell.grid_pos;

      const bool early_x = grip_pos.x == 0;
      const bool early_y = grip_pos.y == 0;
      const bool early_z = grip_pos.z == 0;

      const bool late_x = grip_pos.x == m_cell_count.x - 1;
      const bool late_y = grip_pos.y == m_cell_count.y - 1;
      const bool late_z = grip_pos.z == m_cell_count.z - 1;

      util::dynamic_array<entt::entity> entities;

      for (auto x : vi::ints(0u, 3u))
      {
         if (!(early_x && x == 0u) || !(late_x && x == 2u))
         {
            for (auto y : vi::ints(0u, 3u))
            {
               if (!(early_y && y == 0u) || !(late_y && y == 2u))
               {
                  for (auto z : vi::ints(0u, 3u))
                  {
                     if (!(early_z && z == 0u) || !(late_z && z == 2u))
                     {
                        // m_logger.info("neighbour cell at {}", glm::vec3{x, y, z});

                        for (const auto& inner : m_cells)
                        {
                           if (inner.grid_pos.x == x && inner.grid_pos.y == y &&
                               inner.grid_pos.z == z)
                           {
                              for (auto e : inner.m_entities)
                              {
                                 entities.push_back(e);
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }

      return entities;
   }
} // namespace sph
