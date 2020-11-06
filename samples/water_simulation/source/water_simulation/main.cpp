#include <water_simulation/shader_codex.hpp>

#include <gfx/data_types.hpp>
#include <gfx/render_manager.hpp>
#include <gfx/window.hpp>

#include <ui/window.hpp>

#include <glm/ext/matrix_transform.hpp>

util::dynamic_array<gfx::vertex> m_triangle_vertices{{{-0.5F, -0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                     {{0.5F, -0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                     {{0.5F, 0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                     {{-0.5F, 0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}}};

util::dynamic_array<std::uint32_t> m_triangle_indices{0, 1, 2, 2, 3, 0};

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("vermillon");

   glfwInit();
   glslang::InitializeProcess();

   ui::window window{"Engine", 1080, 720};
   gfx::render_manager rendering_manager{window, main_logger};

   shader_codex codex{rendering_manager, main_logger};
   auto vert_res = codex.insert("resources/shaders/test_vert.spv", vkn::shader_type::vertex);
   auto frag_res = codex.insert("resources/shaders/test_frag.spv", vkn::shader_type::fragment);

   if (auto err = vert_res.error())
   {
      util::log_error(main_logger, "error: {}", err->value().message());
   }

   if (auto err = vert_res.error())
   {
      util::log_error(main_logger, "error: {}", err->value().message());
   }

   const auto vert_shader_info = vert_res.value().value();
   const auto frag_shader_info = frag_res.value().value();

   rendering_manager.bake(vert_shader_info.value(), frag_shader_info.value());

   rendering_manager.subscribe_renderable(
      "rectangle",
      {.vertices = m_triangle_vertices, .indices = m_triangle_indices, .model = glm::mat4{1}});

   while (window.is_open())
   {
      window.poll_events();

      static auto startTime = std::chrono::high_resolution_clock::now();

      auto currentTime = std::chrono::high_resolution_clock::now();
      float time =
         std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime)
            .count();

      auto triangle_0_model =
         glm::rotate(glm::mat4{1.0F}, time * glm::radians(90.0F), glm::vec3(0.0F, 0.0F, 1.0F));
      rendering_manager.update_model_matrix("rectangle", triangle_0_model);

      rendering_manager.render_frame();
   }

   rendering_manager.wait();

   return 0;
}

/*
core::initialize(main_logger);



gfx::context rendering_ctx{main_logger};
gfx::window rendering_wnd{"Engine", 1080, 720}; // NOLINT
gfx::render_manager rendering_manager{rendering_ctx, rendering_wnd, main_logger};

while (rendering_wnd.is_open())
{
   rendering_wnd.poll_events();

   rendering_manager.render_frame();
}

rendering_manager.wait();
*/
