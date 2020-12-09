#include <water_simulation/sph/grid.hpp>

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/remove.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <glm/vec2.hpp>

#include <easy/profiler.h>

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

      m_cell_count = glm::i64vec3{std::ceil(m_dimensions.x * 2 / m_cell_size),  // NOLINT
                                  std::ceil(m_dimensions.y * 2 / m_cell_size),  // NOLINT
                                  std::ceil(m_dimensions.z * 2 / m_cell_size)}; // NOLINT

      // NOLINTNEXTLINE
      m_cells.reserve(static_cast<std::size_t>(m_cell_count.x + m_cell_count.y + m_cell_count.z));

      for (std::int64_t z : vi::ints(0l, m_cell_count.z)) // NOLINT
      {
         for (std::int64_t y : vi::ints(0l, m_cell_count.y)) // NOLINT
         {
            for (std::int64_t x : vi::iota(0l, m_cell_count.x)) // NOLINT
            {
               glm::vec3 center{
                  -m_dimensions.x - half(cell_size) + static_cast<float>(x) * cell_size,
                  -m_dimensions.y - half(cell_size) + static_cast<float>(y) * cell_size,
                  -m_dimensions.z - half(cell_size) + static_cast<float>(z) * cell_size};
               glm::u64vec3 grid_pos = {x, y, z};

               m_cells.push_back({.grid_pos = grid_pos, .center = center});
            }
         }
      }

      m_logger.info("Grid constructed with a total of {} cells distributed as such:\n\t"
                    "x-axis = {}\n\ty-axis = {}\n\tz-axis = {}",
                    m_cell_count.x * m_cell_count.y * m_cell_count.z, m_cell_count.x, // NOLINT
                    m_cell_count.y, m_cell_count.z);                                  // NOLINT
   }

   void grid::insert_particle(particle* p_particle)
   {
      const auto half_size = half(m_cell_size);
      const auto dimensions = glm::vec3{1.0f, 1.0f, 1.0f} * half_size;

      for (cell& c : m_cells)
      {
         if (within_box({c.center, dimensions}, p_particle->position))
         {
            c.particles.push_back(p_particle);
         }
      }
   }

   void grid::update_layout()
   {
      EASY_FUNCTION(profiler::colors::Brick);

      const auto half_size = half(m_cell_size);
      const auto dimensions = glm::vec3{1.0f, 1.0f, 1.0f} * half_size;

      for (auto& cell : m_cells)
      {
         for (auto* p : cell.particles)
         {
            if (!within_box({cell.center, dimensions}, p->position))
            {
               const auto distance = p->position - cell.center;
               const auto adjusted = glm::vec3{
                  is_negative(distance.x) ? distance.x + half_size : distance.x - half_size,
                  is_negative(distance.y) ? distance.y + half_size : distance.y - half_size,
                  is_negative(distance.z) ? distance.z + half_size : distance.z - half_size};
               const auto offset = glm::i64vec3{
                  static_cast<std::int64_t>(is_negative(distance.x) ? std::floor(adjusted.x)
                                                                    : std::ceil(adjusted.x)),
                  static_cast<std::int64_t>(is_negative(distance.y) ? std::floor(adjusted.y)
                                                                    : std::ceil(adjusted.y)),
                  static_cast<std::int64_t>(is_negative(distance.z) ? std::floor(adjusted.z)
                                                                    : std::ceil(adjusted.z))};
               const std::int64_t index =
                  offset.x + offset.y * m_cell_count.x + offset.z * m_cell_count.x * m_cell_count.y;

               p->grid_position = offset;

               m_cells[static_cast<std::size_t>(index)].particles.push_back(p);

               p = nullptr;
            }
         }

         auto begin = std::begin(cell.particles);
         auto end = std::end(cell.particles);
         cell.particles.erase(std::remove(begin, end, nullptr), end);
      }

      /*

            EASY_BLOCK("Clearing");
            parallel_for(m_cells, [](auto& cell) {
               cell.particles.clear();
            });
            EASY_END_BLOCK;

            EASY_BLOCK("Placing");

            for (auto& p : particles)
            {
               auto it = ranges::find_if(m_cells, [&](grid::cell& c) {
                  return within_box({c.center, glm::vec3{1.0f, 1.0f, 1.0f} *
         half(m_cell_size)}, p.position);
               });

               if (it != std::end(m_cells))
               {
                  p.grid_position = it->grid_pos;

                  it->particles.push_back(&p);
               }
            }
            EASY_END_BLOCK;
            */
   }

   auto grid::cells() -> std::span<cell> { return m_cells; }

   auto grid::lookup_neighbours(const glm::i64vec3& grid_pos) -> util::dynamic_array<particle*>
   {
      const int early_x = (grid_pos.x == 0) ? 0 : -1; // NOLINT
      const int early_y = (grid_pos.y == 0) ? 0 : -1; // NOLINT
      const int early_z = (grid_pos.z == 0) ? 0 : -1; // NOLINT

      const int late_x = (grid_pos.x == m_cell_count.x - 1) ? 1 : 2; // NOLINT
      const int late_y = (grid_pos.y == m_cell_count.y - 1) ? 1 : 2; // NOLINT
      const int late_z = (grid_pos.z == m_cell_count.z - 1) ? 1 : 2; // NOLINT

      util::dynamic_array<particle*> particles{};

      for (int x : vi::ints(early_x, late_x))
      {
         for (int y : vi::ints(early_y, late_y))
         {
            for (int z : vi::ints(early_z, late_z))
            {
               glm::i64vec3 adjusted_pos{grid_pos.x + x, grid_pos.y + y, grid_pos.z + z};

               std::int64_t offset = adjusted_pos.x + adjusted_pos.y * m_cell_count.x +
                  adjusted_pos.z * m_cell_count.x * m_cell_count.y;

               auto& c = m_cells[static_cast<std::size_t>(offset)];

               particles.insert(std::cend(particles), c.particles.begin(), c.particles.end());
            }
         }
      }

      return particles;
   }
} // namespace sph
