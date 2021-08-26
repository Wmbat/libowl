#pragma once

#include <sph-simulation/components.hpp>
#include <sph-simulation/core.hpp>
#include <sph-simulation/physics/components.hpp>
#include <sph-simulation/sim_variables.hpp>
#include <sph-simulation/sph/components.hpp>

#include <entt/entt.hpp>

namespace sph
{
   using particle_view =
      entt::view<entt::exclude_t<>, render::component::transform, physics::component::rigid_body,
                 physics::component::sphere_collider, sph::component::particle_data>;

   void update(particle_view& test, const settings& variables, duration<float> time_step);
} // namespace sph
