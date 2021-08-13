#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/sph/grid.hpp>

namespace sph
{
   /**
    * @brief Class used for the handling of all entities that implement the
    * `sph::component::particle` component
    */
   class system
   {
   public:
      struct create_info
      {
         util::non_null<entt::registry*> p_registry;
         util::non_null<util::logger*> p_logger;

         glm::vec3 center;
         glm::vec3 dimensions;

         settings system_settings;
      };

   public:
      system() = default;
      system(create_info&& info);

      /**
       * @brief Add a new particle to the `system`
       *
       * @param particle   The initial state of the particle
       */
      void emit(particle&& particle);

      auto particles() -> std::span<particle>;

      /**
       * @brief performs computations on all entities with the `component::particle` component using
       * a grid space partitioning system and updates the grid once all the computation is done.
       *
       * @param time_step The value of time to update the system by.
       */
      void update(duration<float> time_step);

   private:
      /**
       * @brief Compute all the density and pressure forces of particles within the system.
       */
      void compute_density_pressure();
      /**
       * @brief Compute all the surface normals of particles within the system for later surface
       * tension computations.
       */
      void compute_normals();
      /**
       * @brief Compute the forces acting on particles such as pressure forces, viscosity forces
       * gravity forces, and surface tension forces (cohesion force & curvature force).
       */
      void compute_forces();
      /**
       * @brief Update the velocity and position of the particles using an backward (implicit) euler
       * method
       */
      void integrate(duration<float> time_step);

   private:
      entt::registry* mp_registry{nullptr};

      util::log_ptr m_logger{};

      settings m_settings;

      float m_kernel_radius{};

      grid m_grid;
      std::vector<particle> m_particles;
   };
} // namespace sph
