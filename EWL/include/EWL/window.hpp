#pragma once

#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#endif

#include <EML/stack_allocator.hpp>
#include <EGL/render_manager.hpp>

namespace EWL
{
   class window
   {
   public:
      window( );

   private:
#if defined(VK_USE_PLATFORM_XCB_KHR)
      xcb_connection_t* p_connection;
#endif
   };
}
