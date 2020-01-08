#pragma once

#include <UVE/vk/core.hpp>

#include <memory>
#include <vector>

class logger;

namespace UVE
{
   class render_target;
   class window;

   class context
   {
   public:
      context( );
      context( logger* p_logger );
      context( context const& other ) = delete;
      context( context&& other );
      ~context( );

      context& operator=( context const& rhs ) = delete;
      context& operator=( context&& rhs );

   public:
      std::unique_ptr<render_target> create_render_target( window const& window );

      void find_best_physical_device( std::unique_ptr<render_target> const& render_target );
   private:
      void init_volk( );

   private:
      inline static bool IS_VOLK_INITIALIZED = false;

      VkInstance instance;
      VkDebugUtilsMessengerEXT debug_messenger;

      std::vector<VkPhysicalDevice> available_physical_devices;

      logger* p_logger;
   }; // class context
} // namespace UVE
