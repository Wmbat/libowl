/**
 * @mainpage Epona
 */

#include <core/core.hpp>

#include <gfx/context.hpp>
#include <gfx/data_types.hpp>
#include <gfx/render_manager.hpp>
#include <gfx/window.hpp>

#include <util/logger.hpp>

#include <glm/gtc/matrix_transform.hpp>

util::dynamic_array<gfx::vertex> m_triangle_vertices{{{-0.5F, -0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                     {{0.5F, -0.5F, 0.0F}, {0.0F, 1.0F, 0.0F}},
                                                     {{0.5F, 0.5F, 0.0F}, {0.0F, 0.0F, 1.0F}},
                                                     {{-0.5F, 0.5F, 0.0F}, {1.0F, 1.0F, 1.0F}}};

util::dynamic_array<std::uint32_t> m_triangle_indices{0, 1, 2, 2, 3, 0};

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("vermillon");

   core::initialize(main_logger);

   gfx::context rendering_ctx{main_logger};
   gfx::window rendering_wnd{"Engine", 1080, 720}; // NOLINT
   gfx::render_manager rendering_manager{rendering_ctx, rendering_wnd, main_logger};

   rendering_manager.add_vertex_buffer(m_triangle_vertices);
   rendering_manager.add_index_buffer(m_triangle_indices);
   rendering_manager.bake();

   while (rendering_wnd.is_open())
   {
      rendering_wnd.poll_events();

      rendering_manager.render_frame();
   }

   rendering_manager.wait();

   return 0;
}
