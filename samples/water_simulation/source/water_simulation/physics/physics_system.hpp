#pragma once

#include <water_simulation/particle.hpp>

#include <util/containers/dynamic_array.hpp>

class particle_system
{
public:
private:
   util::dynamic_array<particle> m_particles{};
};
