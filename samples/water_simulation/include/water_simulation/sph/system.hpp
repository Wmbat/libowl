#pragma once

#include <water_simulation/sph/grid.hpp>

namespace sph
{
   /**
    * @brief encapsulate all components within the `sph` submodule
    */
   namespace component
   {
      /**
       * @brief Compenent used for the handling of particles affected by the `sph::system`
       */
      struct particle
      {
         glm::vec3 position{};
         glm::vec3 velocity{};
         glm::vec3 force{};
         glm::vec3 normal{};

         float radius{1.0f};
         float mass{1.0F};
         float density{0.0F};
         float pressure{0.0F};
         float restitution{0.5f};
      };
   } // namespace component

   /**
    * @brief Class used for the handling of all entities that implement the
    * `sph::component::particle` component
    */
   class system
   {
   public:
      struct create_info
      {
         entt::registry* p_registry;

         glm::vec3 center;
         glm::vec3 dimensions;

         float kernel_radius;
         float rest_density;

         util::logger_ptr logger;
      };

   public:
      system() = default;
      system(const create_info& info);

      /**
       * @brief performs computations on all entities with the `component::particle` component using
       * a grid space partitioning system and updates the grid once all the computation is done.
       *
       * @param time_step The value of time to update the system by.
       */
      void update(float time_step);

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
      void integrate();

   private:
      [[maybe_unused]] entt::registry* mp_registry{nullptr};

      [[maybe_unused]] grid m_grid{};

      [[maybe_unused]] float m_kernel_radius{};
      [[maybe_unused]] float m_rest_density{};

      [[maybe_unused]] util::logger_ptr m_logger;
   };
} // namespace sph
