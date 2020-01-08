#pragma once

#include <UVE/vk/core.hpp>

#include <vector>

class logger;

namespace UVE
{
   class render_target
   {
   public:
      render_target( );
      render_target( VkInstance instance, logger* p_logger );
      render_target( render_target const& other ) = delete;
      render_target( render_target&& other );
      ~render_target( );

      render_target& operator=( render_target const& rhs ) = delete;
      render_target& operator=( render_target&& rhs );

      VkSurfaceCapabilitiesKHR get_capabilities( VkPhysicalDevice device );

      std::vector<VkSurfaceFormatKHR> get_formats( VkPhysicalDevice device );
      std::vector<VkPresentModeKHR> get_present_modes( VkPhysicalDevice device );

   protected:
      VkInstance instance;
      VkSurfaceKHR surface;

      logger* p_logger;
   }; // class render_target
} // namespace UVE
