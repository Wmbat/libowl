/**
 * @mainpage Epona
 */

#include "epona_core/core.hpp"
#include "epona_core/detail/logger.hpp"
#include "epona_core/memory/pool_allocator.hpp"
#include "epona_core/render_manager.hpp"

#include <map>

auto main() -> int
{
   core::logger main_logger{"epona_core"};

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
