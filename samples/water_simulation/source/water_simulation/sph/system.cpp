#include <water_simulation/sph/system.hpp>

namespace sph
{
   system::system(const create_info& info) :
      mp_registry{info.p_registry}, m_grid{info.kernel_radius, info.center, info.dimensions},
      m_kernel_radius{info.kernel_radius}, m_rest_density{info.rest_density}
   {}

   void system::update([[maybe_unused]] float time_step) {}

   void system::compute_density() {}
   void system::compute_normals() {}
   void system::compute_forces() {}
   void system::integrate() {}
} // namespace sph
