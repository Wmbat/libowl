/**
 * @file sph/collision/contact.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Saturday, 31st of august 2021
 * @brief 
 * @copyright Copyright (C) 2021 wmbat.
 */


#ifndef SPH_SIMULATION_SPH_COLLISION_CONTACT_HPP
#define SPH_SIMULATION_SPH_COLLISION_CONTACT_HPP

#include <sph-simulation/physics/rigid_body.hpp>
#include <sph-simulation/sph/particle.hpp>
#include <sph-simulation/transform.hpp>

#include <glm/ext/vector_float3.hpp>

#include <array>

namespace sph 
{
   /**
    * @brief
    */
   struct contact
   {
      glm::vec3 point;
      glm::vec3 normal;

      float penetration_depth;
      float friction;
      float restitution;

      particle* p_particle;
      ::transform* p_particle_transform;

      physics::rigid_body* p_rigid_body;
      ::transform* p_rigid_body_transform;
   };
} // namespace sph

#endif // SPH_SIMULATION_SPH_COLLISION_CONTACT_HPP
