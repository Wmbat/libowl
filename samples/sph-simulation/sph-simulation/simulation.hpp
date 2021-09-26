#pragma once

#include <sph-simulation/sim_config.hpp>

#include <libmannele/logging/log_ptr.hpp>

struct simulation_info
{
   sim_config config;

   mannele::log_ptr logger;    
};

auto start_simulation(const simulation_info& info) -> int;
