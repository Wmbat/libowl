/**
 * @mainpage Epona
 */

#include <core/core.hpp>

#include <gfx/data_types.hpp>
#include <gfx/render_manager.hpp>
#include <gfx/window.hpp>

#include <gfx/stb_image.h>

#include <util/logger.hpp>

#include <glm/gtc/matrix_transform.hpp>

util::dynamic_array<gfx::vertex> m_triangle_0_vertices{{{-0.5F, -0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                       {{0.5F, -0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                       {{0.5F, 0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                       {{-0.5F, 0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}}};

util::dynamic_array<gfx::vertex> m_triangle_1_vertices{{{-0.5F, -0.5F, 0.0F}, {0.0F, 1.0F, 0.0F}},
                                                       {{0.5F, -0.5F, 0.0F}, {0.0F, 1.0F, 0.0F}},
                                                       {{0.5F, 0.5F, 0.0F}, {0.0F, 1.0F, 0.0F}},
                                                       {{-0.5F, 0.5F, 0.0F}, {0.0F, 1.0F, 0.0F}}};

util::dynamic_array<std::uint32_t> m_triangle_indices{0, 1, 2, 2, 3, 0};

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("vermillon");

   core::initialize(main_logger);

   ui::window rendering_wnd{"Engine", 1080, 720}; // NOLINT
   gfx::render_manager rendering_manager{rendering_wnd, main_logger};

   rendering_manager.subscribe_renderable(
      "triangle_0",
      {.vertices = m_triangle_0_vertices, .indices = m_triangle_indices, .model = glm::mat4{1}});

   rendering_manager.subscribe_renderable(
      "triangle_1",
      {.vertices = m_triangle_1_vertices, .indices = m_triangle_indices, .model = glm::mat4{1}});

   //rendering_manager.bake();

   while (rendering_wnd.is_open())
   {
      rendering_wnd.poll_events();

      static auto startTime = std::chrono::high_resolution_clock::now();

      auto currentTime = std::chrono::high_resolution_clock::now();
      float time =
         std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime)
            .count();

      auto triangle_0_model =
         glm::rotate(glm::mat4{1.0F}, time * glm::radians(90.0F), glm::vec3(0.0F, 0.0F, 1.0F));
      rendering_manager.update_model_matrix("triangle_0", triangle_0_model);

      auto triangle_1_model = glm::translate(
         glm::rotate(glm::mat4{1.0F}, time * glm::radians(90.0F), glm::vec3{0.0F, 0.0F, 1.0F}),
         glm::vec3{0.0F, 0.0F, 1.0F});
      rendering_manager.update_model_matrix("triangle_1", triangle_1_model);

      rendering_manager.render_frame();
   }

   rendering_manager.wait();

   return 0;
}
