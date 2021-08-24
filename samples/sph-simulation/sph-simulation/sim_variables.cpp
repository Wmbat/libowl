#include <sph-simulation/sim_variables.hpp>

auto compute_kernel_radius(const sim_variables& variables) -> float
{
   return variables.water_radius * variables.kernel_multiplier;
}
