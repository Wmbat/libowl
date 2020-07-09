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
   util::logger main_logger{"epona_core"};

   core::initialize(&main_logger);

   core::gfx::window main_window{"Engine", 1080, 720};
   core::render_manager render_manager{&main_window, &main_logger};

   /*
   while (main_window.is_open())
   {
   }
   */

   return 0;
}
