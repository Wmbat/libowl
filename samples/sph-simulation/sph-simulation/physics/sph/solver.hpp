#ifndef SPH_SIMULATION_PHYSICS_SPH_SOLVER_HPP
#define SPH_SIMULATION_PHYSICS_SPH_SOLVER_HPP

#include <sph-simulation/components.hpp>
#include <sph-simulation/core.hpp>
#include <sph-simulation/physics/collision/colliders.hpp>
#include <sph-simulation/physics/sph/particle.hpp>
#include <sph-simulation/sim_variables.hpp>

#include <entt/entt.hpp>

#define PARTICLE_COMPONENTS                                                                        \
   render::component::transform, physics::sphere_collider, physics::sph::particle

namespace physics::sph
{
   using particle_view = entt::view<entt::exclude_t<>, PARTICLE_COMPONENTS>;

   void solve(const particle_view& particles, const sim_variables& variables, duration<float> time_step);
} // namespace physics::sph

#endif // SPH_SIMULATION_PHYSICS_SPH_SOLVER_HPP
