#ifndef SPH_SIMULATION_SIM_CONFIG_HPP
#define SPH_SIMULATION_SIM_CONFIG_HPP

#include <sph-simulation/sim_variables.hpp>

#include <libmannele/dimension.hpp>

#include <chrono>
#include <string>

struct sim_config 
{
   std::string name;

   bool is_onscreen_rendering_enabled;
   bool is_offscreen_rendering_enabled;

   mannele::dimension_u32 dimensions;
   mannele::u32 frame_count;

   std::chrono::duration<float, std::milli> time_step;

   sim_variables variables;
};

#endif // SPH_SIMULATION_SIM_CONFIG_HPP
