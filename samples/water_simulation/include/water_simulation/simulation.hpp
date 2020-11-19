#pragma once

#include <water_simulation/core.hpp>
#include <water_simulation/kernel.hpp>
#include <water_simulation/particle.hpp>

#include <util/containers/dynamic_array.hpp>

class simulation
{
public:
   simulation(const settings& settings);

private:
   settings m_settings;

   util::dynamic_array<particle> particles;
};
