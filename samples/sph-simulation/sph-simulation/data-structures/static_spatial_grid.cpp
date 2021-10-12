#include "libmannele/core.hpp"
#include <sph-simulation/data-structures/static_spatial_grid.hpp>

#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip.hpp>

#include <libmannele/maths/maths.hpp>

namespace rv = ranges::views;

using mannele::f32;
using mannele::u64;

static constexpr u64 grid_edge_buffer = 2;

static_spatial_grid::static_spatial_grid(const glm::vec2& bounds_x, const glm::vec2& bounds_y,
                                         const glm::vec2& bounds_z, mannele::f32 unit_size) :
   m_bounds_x(bounds_x),
   m_bounds_y(bounds_y), m_bounds_z(bounds_z), m_unit_size(unit_size)
{
   const f32 full_length_x = std::abs(bounds_x.x) + std::abs(bounds_x.y); // NOLINT
   const f32 full_length_y = std::abs(bounds_y.x) + std::abs(bounds_y.y); // NOLINT
   const f32 full_length_z = std::abs(bounds_z.x) + std::abs(bounds_z.y); // NOLINT

   const auto unit_count_x =
      static_cast<u64>(std::ceil(full_length_x / m_unit_size)) + grid_edge_buffer; // NOLINT
   const auto unit_count_y =
      static_cast<u64>(std::ceil(full_length_y / m_unit_size)) + grid_edge_buffer; // NOLINT
   const auto unit_count_z =
      static_cast<u64>(std::ceil(full_length_z / m_unit_size)) + grid_edge_buffer; // NOLINT

   m_units.reserve(unit_count_x * unit_count_y * unit_count_z);

   for (u64 grid_x : rv::iota(0u, unit_count_x))
   {
      for (u64 grid_y : rv::iota(0u, unit_count_y))
      {
         for (u64 grid_z : rv::iota(0u, unit_count_z))
         {
            f32 center_x = bounds_x.x - mannele::half(unit_size) + static_cast<f32>(grid_x);
            f32 center_y = bounds_y.x - mannele::half(unit_size) + static_cast<f32>(grid_y);
            f32 center_z = bounds_z.x - mannele::half(unit_size) + static_cast<f32>(grid_z);

            m_units.push_back(
               {.grid_offset = {grid_x, grid_y, grid_z}, .half_size = mannele::half(unit_size)});
         }
      }
   }
}
