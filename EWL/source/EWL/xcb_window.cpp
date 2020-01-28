#include <EWL/window.hpp>

#if defined( VK_USE_PLATFORM_XCB_KHR )
#   include <xcb/xproto.h>

namespace EWL
{
   window::window( ) : p_connection( xcb_connect( nullptr, nullptr ) )
   {
      xcb_setup_t const* p_setup = xcb_get_setup( p_connection );
      xcb_screen_iterator_t it = xcb_setup_roots_iterator( p_setup );
      xcb_screen_t* p_screen = it.data;

      xcb_window_t window = xcb_generate_id( p_connection );
      xcb_create_window( p_connection, XCB_COPY_FROM_PARENT, window, p_screen->root, 0, 0, 100, 100, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT,
         p_screen->root_visual, 0, NULL );
   }
} // namespace EWL

#endif
