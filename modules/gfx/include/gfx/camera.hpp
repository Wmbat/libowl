#pragma once

#include <gfx/data_types.hpp>

#include <glm/glm.hpp>

namespace core
{
   class camera
   {
   public:
      camera() = default;
      camera(const glm::vec3& position, float fov, float aspect, float near_plane,
             float far_plane) noexcept;

   private:
      void update_view_matrix() noexcept;

   private:
      float m_fov{90.0f};
      float m_near_plane{};
      float m_far_plane{};

      glm::vec3 m_position{};
      glm::vec3 m_rotation{};
      glm::vec4 m_view_pos{};

      bool m_is_updated = false;
      bool m_should_flip_y = false;

      gfx::camera_matrices m_matrices{};
   };
} // namespace core
