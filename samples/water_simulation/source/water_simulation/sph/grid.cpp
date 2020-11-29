#include <water_simulation/sph/grid.hpp>

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <glm/vec2.hpp>

#include <cassert>
#include <execution>

namespace vi = ranges::views;

struct box
{
   glm::vec3 center;
   glm::vec3 dimensions;
};

auto within_box(const box& b, const glm::vec3& p) -> bool
{
   bool within_x = (p.x < b.center.x + b.dimensions.x) && (p.x >= b.center.x - b.dimensions.x);
   bool within_y = (p.y < b.center.y + b.dimensions.y) && (p.y >= b.center.y - b.dimensions.y);
   bool within_z = (p.z < b.center.z + b.dimensions.z) && (p.z >= b.center.z - b.dimensions.z);

   return within_x && within_y && within_z;
}

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

      for (std::size_t z : vi::iota(0u, m_cell_count.z)) // NOLINT
      {
         for (std::size_t y : vi::iota(0u, m_cell_count.y)) // NOLINT
         {
            for (std::size_t x : vi::iota(0u, m_cell_count.x)) // NOLINT
            {
               glm::vec3 center{
                  -m_dimensions.x - half(cell_size) + static_cast<float>(x) * cell_size,
                  -m_dimensions.y - half(cell_size) + static_cast<float>(y) * cell_size,
                  -m_dimensions.z - half(cell_size) + static_cast<float>(z) * cell_size};
               glm::u64vec3 grid_pos = {x, y, z};

               /*
               m_logger.debug(
                  "grid cell:\n\t-> index = ({}, {}, {})\n\t-> center = {}\n\t-> dimensions = {}",
                  grid_pos.x, grid_pos.y, grid_pos.z, cell_size, dims);
                  */

               m_cells.push_back({.grid_pos = grid_pos, .center = center});
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
      for (auto& cell : m_cells)
      {
         cell.entities.clear();
      }

      auto view = p_registry->view<component::particle>();

      std::for_each(std::begin(view), std::end(view), [&](auto e) {
         const auto& particle = p_registry->get<component::particle>(e);

         auto it = ranges::find_if(m_cells, [&](grid::cell& c) {
            return within_box({c.center, {half(m_cell_size), half(m_cell_size), half(m_cell_size)}},
                              particle.position);
         });

         if (it != std::end(m_cells))
         {
            it->entities.push_back(e);
         }
      });
   }

   auto grid::cells() -> std::span<cell> { return m_cells; }

   auto grid::lookup_neighbours(const cell& cell) -> util::dynamic_array<entt::entity>
   {
      const auto& grip_pos = cell.grid_pos;

      const int early_x = (grip_pos.x == 0) ? 0 : -1; // NOLINT
      const int early_y = (grip_pos.y == 0) ? 0 : -1; // NOLINT
      const int early_z = (grip_pos.z == 0) ? 0 : -1; // NOLINT

      const int late_x = (grip_pos.x == m_cell_count.x - 1) ? 1 : 2; // NOLINT
      const int late_y = (grip_pos.y == m_cell_count.y - 1) ? 1 : 2; // NOLINT
      const int late_z = (grip_pos.z == m_cell_count.z - 1) ? 1 : 2; // NOLINT

      util::dynamic_array<entt::entity> entities{};
      entities.reserve(std::size(cell.entities));

      for (auto e : cell.entities)
      {
         entities.push_back(e);
      }

      for (int x : vi::ints(early_x, late_x))
      {
         for (int y : vi::ints(early_y, late_y))
         {
            for (int z : vi::ints(early_z, late_z))
            {
               glm::u64vec3 adjusted_pos{static_cast<int>(grip_pos.x) + x,  // NOLINT
                                         static_cast<int>(grip_pos.y) + y,  // NOLINT
                                         static_cast<int>(grip_pos.z) + z}; // NOLINT

               /*
               m_logger.debug("cell ({}, {}, {}) neighbour -> ({}, {}, {})", grip_pos.x,
               grip_pos.y, grip_pos.z, adjusted_pos.x, adjusted_pos.y, adjusted_pos.z);
               */

               std::size_t offset = adjusted_pos.x + adjusted_pos.y * m_cell_count.x +
                  adjusted_pos.z * m_cell_count.x * m_cell_count.y;

               auto& c = m_cells[offset];

               entities.insert(std::cend(entities), c.entities.begin(), c.entities.end());
            }
         }
      }

      return entities;
   }
} // namespace sph
