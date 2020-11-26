#include <water_simulation/sph/grid.hpp>

#include <range/v3/view/iota.hpp>

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
                    m_cell_count.x + m_cell_count.y + m_cell_count.z, m_cell_count.x, // NOLINT
                    m_cell_count.y, m_cell_count.z);                                  // NOLINT
   }

   auto grid::cells() -> std::span<cell> { return m_cells; }

   auto grid::lookup_neighbours([[maybe_unused]] const cell& cell)
      -> util::dynamic_array<entt::entity>
   {
      return util::dynamic_array<entt::entity>{};
   }
} // namespace sph
