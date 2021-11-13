#ifndef SPH_SIMULATION_DATA_STRUCTURE_FIXED_SPATIAL_GRID_HPP_
#define SPH_SIMULATION_DATA_STRUCTURE_FIXED_SPATIAL_GRID_HPP_

#include <libmannele/core.hpp>

#include <libmannele/maths/maths.hpp>

#include <glm/glm.hpp>

#include <entt/entt.hpp>

#include <range/v3/utility/common_tuple.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip.hpp>

#include <span>
#include <vector>

namespace detail
{
   using mannele::u64;

   auto compute_grid_division(const glm::vec2& bounds_x, const glm::vec2& bounds_y,
                              const glm::vec2& bounds_z, mannele::f32 unit_size) -> glm::u64vec3;
} // namespace detail

class fixed_spatial_grid
{
   using f32 = mannele::f32;
   using u64 = mannele::u64;

public:
   struct unit
   {
      glm::u64vec3 grid_offset{};
      glm::vec3 center{};
      mannele::f32 half_size{};

      std::vector<entt::entity*> particles;
   };

public:
   fixed_spatial_grid(const glm::vec2& bounds_x, const glm::vec2& bounds_y,
                      const glm::vec2& bounds_z, mannele::f32 unit_size);

   void insert(entt::entity* particle) {}

private:
   [[nodiscard]] auto all_grid_positions() const
   {
      namespace rv = ranges::views;
      const auto unit_count =
         detail::compute_grid_division(m_bounds_x, m_bounds_y, m_bounds_z, m_unit_size);

      return rv::zip(rv::iota(0u, unit_count.x),  // NOLINT
                     rv::iota(0u, unit_count.y),  // NOLINT
                     rv::iota(0u, unit_count.z)); // NOLINT
   }

   [[nodiscard]] auto construct_units(const ranges::common_tuple<u64, u64, u64>& index) const
      -> unit;

private:
   glm::vec2 m_bounds_x;
   glm::vec2 m_bounds_y;
   glm::vec2 m_bounds_z;
   mannele::f32 m_unit_size;

   std::vector<unit> m_units;

   static constexpr mannele::u64 grid_edge_buffer = 1;
};

#endif // SPH_SIMULATION_DATA_STRUCTURE_FIXED_SPATIAL_GRID_HPP_
