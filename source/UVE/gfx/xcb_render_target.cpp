#include <UVE/gfx/xcb_render_target.hpp>
#include <UVE/utils/logger.hpp>

namespace UVE
{
#if defined( VK_USE_PLATFORM_XCB_KHR )
   xcb_render_target::xcb_render_target( ) : render_target( ) {}
   xcb_render_target::xcb_render_target( xcb_connection_t* connection, xcb_window_t const& window, VkInstance instance, logger* p_logger ) :
      render_target( instance, p_logger )
   {
      auto create_info = VkXcbSurfaceCreateInfoKHR{ };
      create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
      create_info.pNext = nullptr;
      create_info.flags = 0;
      create_info.connection = connection;
      create_info.window = window;

      auto result = vkCreateXcbSurfaceKHR( instance, &create_info, nullptr, &surface );
      if ( result != VK_SUCCESS )
      {
         LOG_ERROR_P( p_logger, "Failed to create Surface: {1}", vk::result_to_string( result ) ); 
      }
      else
      {
         LOG_INFO( p_logger, "Vulkan Surface Created" );
      }
   }
#endif
} // namespace UVE
