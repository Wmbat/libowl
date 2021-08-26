#ifndef SPH_SIMULATION_PHYSICS_SYSTEM_HPP
#define SPH_SIMULATION_PHYSICS_SYSTEM_HPP

#include <sph-simulation/core.hpp>

#include <entt/entity/fwd.hpp>

namespace physics
{
   void update(entt::registry* p_registry, duration<float> time_step);
} // namespace physics

#endif // SPH_SIMULATION_PHYSICS_SYSTEM_HPP
