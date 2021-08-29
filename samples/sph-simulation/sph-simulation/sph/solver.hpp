#ifndef SPH_SIMULATION_SPH_SOLVER_HPP
#define SPH_SIMULATION_SPH_SOLVER_HPP

#include <sph-simulation/components.hpp>
#include <sph-simulation/core.hpp>
#include <sph-simulation/physics/collision/colliders.hpp>
#include <sph-simulation/sim_variables.hpp>
#include <sph-simulation/sph/particle.hpp>
#include <sph-simulation/transform.hpp>

#include <entt/entt.hpp>

#define PARTICLE_COMPONENTS transform, physics::sphere_collider, sph::particle

namespace sph
{
   /**
    * @brief Alias for all the components that help define a particle in the ECS.
    */
   using particle_view = entt::view<entt::exclude_t<>, PARTICLE_COMPONENTS>;

   /**
    * @brief Solve the SPH equations to simulate the fluid particles
    *
    * @param[in] particles The particles to use for the computations.
    * @param[in] sim_variables The different variables used to define the system.
    * @param[in] time_step The time per iteration of the solver.
    */
   void solve(const particle_view& particles, const sim_variables& variables,
              duration<float> time_step);
} // namespace sph

#endif // SPH_SIMULATION_SPH_SOLVER_HPP
