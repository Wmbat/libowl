#include <UVE/gfx/render_target.hpp>
#include <UVE/utils/logger.hpp>

namespace UVE
{
   render_target::render_target( ) : instance( VK_NULL_HANDLE ), surface( VK_NULL_HANDLE ), p_logger( nullptr ) {}

   render_target::render_target( VkInstance instance, logger* p_logger ) :
      instance( instance ), surface( VK_NULL_HANDLE ), p_logger( nullptr )
   {}

   render_target::render_target( render_target&& other ) { *this = std::move( other ); }

   render_target::~render_target( )
   {
      if ( surface != VK_NULL_HANDLE )
      {
         vkDestroySurfaceKHR( instance, surface, nullptr );
         surface = VK_NULL_HANDLE;
      }

      instance = VK_NULL_HANDLE;

      p_logger = nullptr;
   }

   render_target& render_target::operator=( render_target&& rhs )
   {
      if ( this != &rhs )
      {
         instance = rhs.instance;
         rhs.instance = VK_NULL_HANDLE;

         surface = rhs.surface;
         rhs.surface = VK_NULL_HANDLE;

         p_logger = rhs.p_logger;
         rhs.p_logger = nullptr;
      }

      return *this;
   }

   VkSurfaceCapabilitiesKHR render_target::get_capabilities( VkPhysicalDevice device )
   {
      VkSurfaceCapabilitiesKHR capabilities;

      vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &capabilities );

      return capabilities;
   }
} // namespace UVE
