#include <gfx/data_types.hpp>
#include <gfx/render_manager.hpp>
#include <gfx/window.hpp>

#include <ui/window.hpp>

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("vermillon");

   glfwInit();
   glslang::InitializeProcess();

   ui::window window{"Engine", 1080, 720};
   gfx::render_manager rendering_manager{window, main_logger};

   while (window.is_open())
   {
      window.poll_events();
   }

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
