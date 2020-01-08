#pragma once

#include <UVE/gfx/render_target.hpp>

#if defined( VK_USE_PLATFORM_XCB_KHR )
#   include <xcb/xcb.h>
#endif

namespace UVE
{
#if defined( VK_USE_PLATFORM_XCB_KHR )
   class xcb_render_target : public render_target
   {
   public:
      xcb_render_target( );
      xcb_render_target( xcb_connection_t* connection, xcb_window_t const& window, VkInstance instance, logger* p_logger );
   }; // class xcb_render_target
#endif
} // namespace UVE
