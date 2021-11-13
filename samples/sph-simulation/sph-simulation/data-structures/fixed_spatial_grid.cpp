#include "glm/fwd.hpp"
#include <sph-simulation/data-structures/fixed_spatial_grid.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

namespace rv = ranges::views;

namespace detail
{
   auto compute_grid_division(const glm::vec2& bounds_x, const glm::vec2& bounds_y,
                              const glm::vec2& bounds_z, mannele::f32 unit_size) -> glm::u64vec3
   {}
} // namespace detail

// clang-format off

fixed_spatial_grid::fixed_spatial_grid(const glm::vec2& bounds_x, const glm::vec2& bounds_y,
                                       const glm::vec2& bounds_z, mannele::f32 unit_size) :
   m_bounds_x(bounds_x),
   m_bounds_y(bounds_y), m_bounds_z(bounds_z), m_unit_size(unit_size),
   m_units(all_grid_positions() 
      | rv::transform([this](auto&& t) { return construct_units(t); }) 
      | ranges::to_vector)
{ }
// clang-format on

auto fixed_spatial_grid::construct_units(const ranges::common_tuple<u64, u64, u64>& index) const
   -> unit
{
   const auto [x, y, z] = index;
   const f32 half_unit_width = mannele::half(m_unit_size);

   const f32 center_x = m_bounds_x.x - half_unit_width + static_cast<f32>(x); // NOLINT
   const f32 center_y = m_bounds_y.x - half_unit_width + static_cast<f32>(y); // NOLINT
   const f32 center_z = m_bounds_z.x - half_unit_width + static_cast<f32>(z); // NOLINT

   return {.grid_offset = {x, y, z},
           .center = {center_x, center_y, center_z},
           .half_size = mannele::half(m_unit_size)};
}
