#ifndef SPH_SIMULATION_SPH_SYSTEM_HPP
#define SPH_SIMULATION_SPH_SYSTEM_HPP

#include <sph-simulation/sph/collision/detection.hpp>
#include <sph-simulation/sph/particle.hpp>
#include <sph-simulation/sph/solver.hpp>

namespace sph
{
   /**
    * @brief Data required to update the SPH system
    */
   struct system_update_info
   {
      particle_view particles;
      sphere_view spheres;
      plane_view planes;
      box_view boxes;

      const sim_variables& variables;

      duration<float> time_step;
   };

   /**
    * @brief Updates the SPH system by solving a single iteration of the SPH simulation and handles
    * collisions between particles and physically based rigid bodies.
    *
    * @param[in] info The data required to update the SPH system.
    */
   void update(const system_update_info& info);
} // namespace sph

#endif // SPH_SIMULATION_SPH_SYSTEM_HPP
