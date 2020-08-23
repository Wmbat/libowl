/**
 * @mainpage Epona
 */

#include <core/core.hpp>
#include <core/memory/pool_allocator.hpp>
#include <core/render_manager.hpp>

#include <util/logger.hpp>

#include <map>

auto main() -> int
{
   util::logger main_logger{"mélodie"};

   core::initialize(&main_logger);

   core::gfx::window main_window{"Engine", 1080, 720};
   core::render_manager render_manager{&main_window, &main_logger};

   while (main_window.is_open())
   {
      main_window.poll_events();

      render_manager.render_frame();
   }

   render_manager.wait();

   return 0;
}
