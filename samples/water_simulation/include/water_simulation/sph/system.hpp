#pragma once

#include <water_simulation/sph/grid.hpp>

namespace sph
{
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
      };

   public:
      system() = default;
      system(const create_info& info);

      /**
       * @brief Update whole the system by the specified `time_step`
       *
       * @param time_step The value of time to update the system by.
       */
      void update(float time_step);

   private:
      void compute_density();
      void compute_normals();
      void compute_forces();
      void integrate();

   private:
      [[maybe_unused]] entt::registry* mp_registry{nullptr};

      [[maybe_unused]] grid m_grid{0, {}, {}};

      [[maybe_unused]] float m_kernel_radius{};
      [[maybe_unused]] float m_rest_density{};
   };
} // namespace sph
