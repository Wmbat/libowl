#pragma once

#include <water_simulation/core.hpp>

#include <entt/entity/fwd.hpp>

namespace collision
{
   static constexpr float epsilon = 0.0001f;

   namespace component
   {
      /**
       * @brief Component used for the detection of collisions using a box.
       */
      struct box_collider
      {
         glm::vec3 center;
         glm::vec3 half_size;
      };

      /**
       * @brief Component used for the detection of collisions using a sphere.
       */
      struct sphere_collider
      {
         glm::vec3 center;
         float radius;
      };

   } // namespace component

   /**
    * @brief Class used for the handling of all entities that implement components from
    * the `collision::component` namespace
    */
   class system
   {
   public:
      struct create_info
      {
         vml::non_null<entt::registry*> p_registry;
      };

   public:
      system();
      system(create_info&& info);

      void update(duration<float> time_step);

   private:
      void resolve_particle_to_box_collision(duration<float> time_step);

   private:
      entt::registry* mp_registry;
   };
} // namespace collision
