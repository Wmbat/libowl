#ifndef SPH_SIMULATION_DATA_STRUCTURE_STATIC_SPATIAL_GRID_HPP_
#define SPH_SIMULATION_DATA_STRUCTURE_STATIC_SPATIAL_GRID_HPP_

#include <libmannele/core.hpp>

#include <glm/glm.hpp>

#include <entt/entt.hpp>

#include <span>
#include <vector>

class static_spatial_grid
{
public:
   struct unit
   {
      glm::u64vec3 grid_offset;
      glm::vec3 center;
      mannele::f32 half_size;

      std::vector<entt::entity*> particles;
   };

public:
   static_spatial_grid(const glm::vec2& bounds_x, const glm::vec2& bounds_y,
                       const glm::vec2& bounds_z, mannele::f32 unit_size);

   auto units() -> std::span<unit>;

private:
   glm::vec2 m_bounds_x;
   glm::vec2 m_bounds_y;
   glm::vec2 m_bounds_z;
   mannele::f32 m_unit_size;

   std::vector<unit> m_units;
};

#endif // SPH_SIMULATION_DATA_STRUCTURE_STATIC_SPATIAL_GRID_HPP_
