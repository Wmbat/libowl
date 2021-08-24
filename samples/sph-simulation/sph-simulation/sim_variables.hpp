#ifndef SPH_SIMULATION_SIM_VARIABLES_HPP
#define SPH_SIMULATION_SIM_VARIABLES_HPP

struct sim_variables
{
   float gas_constant;
   float rest_density;
   float viscosity_constant;
   float surface_tension_coefficient;
   float gravity_multiplier;
   float kernel_multiplier;

   float water_radius;
   float water_mass;
};

auto compute_kernel_radius(const sim_variables& variables) -> float;

#endif // SPH_SIMULATION_SIM_VARIABLES_HPP
