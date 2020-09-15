#include <gfx/camera.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace core
{
   camera::camera(const glm::vec3& position, float fov, float aspect, float near_plane,
                  float far_plane) noexcept :
      m_fov{fov},
      m_near_plane{near_plane}, m_far_plane{far_plane}, m_position{position}
   {
      m_matrices.perspective = glm::perspective(m_fov, aspect, m_near_plane, m_far_plane);

      if (m_should_flip_y)
      {
         m_matrices.perspective[1][1] *= -1.0f;
      }

      update_view_matrix();
   }

   void camera::update_view_matrix() noexcept
   {
      glm::mat4 rot_matrix = glm::mat4{1.0f};

      rot_matrix =
         glm::rotate(rot_matrix, glm::radians(m_rotation.x * (m_should_flip_y ? -1.0f : 1.0f)),
                     glm::vec3{1.0f, 0.0f, 0.0f});
      rot_matrix = glm::rotate(rot_matrix, glm::radians(m_rotation.y), glm::vec3{0.0f, 1.0f, 0.0f});
      rot_matrix = glm::rotate(rot_matrix, glm::radians(m_rotation.z), glm::vec3{0.0f, 0.0f, 1.0f});

      glm::vec3 translation = m_position;
      if (m_should_flip_y)
      {
         translation.y *= -1.0f;
      }

      m_matrices.view = glm::translate(glm::mat4{1.0f}, translation);
      m_view_pos = glm::vec4{m_position, 0.0f} * glm::vec4{-1.0f, 1.0f, -1.0f, 1.0f};

      m_is_updated = true;
   }
} // namespace core
