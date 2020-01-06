#include <UVE/core/common_types.hpp>
#include <UVE/core/context.hpp>
#include <UVE/core/core.hpp>
#include <UVE/utils/logger.hpp>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback( VkDebugUtilsMessageSeverityFlagBitsEXT message_severity_bits,
   VkDebugUtilsMessageTypeFlagBitsEXT message_type_bits, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data )
{
   if ( message_severity_bits == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
   {
      spdlog::info( "{0}", p_callback_data->pMessage );
   }
   else if ( message_severity_bits == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
   {
      spdlog::warn( "{0}", p_callback_data->pMessage );
   }
   else if ( message_severity_bits == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
   {
      spdlog::error( "{0}", p_callback_data->pMessage );
   }

   return VK_FALSE;
}

namespace UVE
{
   context::context( ) :
      instance( VK_NULL_HANDLE ), debug_messenger( VK_NULL_HANDLE ), physical_device( VK_NULL_HANDLE ), logical_device( VK_NULL_HANDLE ),
      p_logger( nullptr )
   {
      init_volk( );
   }

   context::context( logger* p_logger ) :
      instance( VK_NULL_HANDLE ), debug_messenger( VK_NULL_HANDLE ), physical_device( VK_NULL_HANDLE ), logical_device( VK_NULL_HANDLE ),
      p_logger( p_logger )
   {
      init_volk( );

      uint32 api_version = 0;
      vkEnumerateInstanceVersion( &api_version );

      LOG_INFO_P( p_logger, "Vulkan API version: {1}, {2}, {3}", VK_VERSION_MAJOR( api_version ), VK_VERSION_MINOR( api_version ),
         VK_VERSION_PATCH( api_version ) );

      auto app_info = VkApplicationInfo{};
      app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pNext = nullptr;
      app_info.apiVersion = api_version;
      app_info.engineVersion = VK_MAKE_VERSION( ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH );
      app_info.pEngineName = ENGINE_NAME;
      app_info.applicationVersion = VK_MAKE_VERSION( 0, 0, 0 );
      app_info.pApplicationName = nullptr;

      auto instance_create_info = VkInstanceCreateInfo{ };
      instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   }

   void context::init_volk( )
   {
      if ( !IS_VOLK_INITIALIZED )
      {
         auto result = volkInitialize( );
         if ( result != VK_SUCCESS )
         {
            LOG_ERROR_P( p_logger, "Failed to initialize volk: {1}", vk::result_to_string( result ) );
         }
         else
         {
            LOG_INFO( p_logger, "Volk initialized successfully" );
         }

         IS_VOLK_INITIALIZED = true;
      }
   }
} // namespace UVE
