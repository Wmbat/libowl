#include <UVE/core/context.hpp>
#include <UVE/utils/logger.hpp>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback( VkDebugUtilsMessageSeverityFlagBitsEXT message_severity_bits,
   VkDebugUtilsMessageTypeFlagBitsEXT message_type_bits, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data )
{
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
   }

   void context::init_volk( )
   {
      if ( !IS_VOLK_INITIALIZED )
      {
         auto result = volkInitialize( );
         if ( result != VK_SUCCESS )
         {
            if ( p_logger )
            {
               p_logger->error( "[{0}] Failed to initialize volk: {1}", __FUNCTION__, vk::result_to_string( result ) );
            }
         }
         else
         {
            if ( p_logger )
            {
               p_logger->info( "[{0}] Volk initialized successfully", __FUNCTION__ );
            }
         }

         IS_VOLK_INITIALIZED = true;
      }
   }
} // namespace UVE
