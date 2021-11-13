#ifndef SPH_SIMULATION_SPH_COLLISION_DETECTION_HPP
#define SPH_SIMULATION_SPH_COLLISION_DETECTION_HPP

#include <sph-simulation/sph/collision/contact.hpp>
#include <sph-simulation/sph/particle.hpp>
#include <sph-simulation/sph/solver.hpp>

#include <sph-simulation/physics/system.hpp>

#include <vector>

namespace sph
{
   using sphere_view = physics::sphere_view;
   using box_view = physics::box_view;
   using plane_view = physics::plane_view;

   /**
    * @brief Find all particles that have collided with a plane.
    *
    * @param[in] particles
    * @param[in] planes
    *
    * @return A vector of all the collisions that have taken place
    */
   auto detect_particle_and_plane_collision(const particle_view& particles,
                                            const plane_view& planes) -> std::vector<contact>;
} // namespace sph

#endif // SPH_SIMULATION_SPH_COLLISION_DETECTION_HPP
