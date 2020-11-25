#include <water_simulation/sph/system.hpp>

namespace sph
{
   system::system(const create_info& info) :
      mp_registry{info.p_registry}, m_grid{info.kernel_radius, info.dimensions, info.logger},
      m_kernel_radius{info.kernel_radius}, m_rest_density{info.rest_density}, m_logger{info.logger}
   {}

   void system::update([[maybe_unused]] float time_step)
   {
      for ([[maybe_unused]] auto& cell : m_grid.cells())
      {
      }
   }

   void system::compute_density_pressure() {}
   void system::compute_normals() {}
   void system::compute_forces() {}
   void system::integrate() {}
} // namespace sph
