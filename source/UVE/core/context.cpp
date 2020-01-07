#include <UVE/core/common_types.hpp>
#include <UVE/core/context.hpp>
#include <UVE/core/core.hpp>
#include <UVE/utils/logger.hpp>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
   VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data )
{
   if ( message_type != 0 )
   {
   }

   if ( p_user_data != nullptr )
   {
   }

   if ( message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
   {
      spdlog::info( "{0}", p_callback_data->pMessage );
   }
   else if ( message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
   {
      spdlog::warn( "{0}", p_callback_data->pMessage );
   }
   else if ( message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
   {
      spdlog::error( "{0}", p_callback_data->pMessage );
   }

   return VK_FALSE;
}

namespace UVE
{
   context::context( ) : instance( VK_NULL_HANDLE ), debug_messenger( VK_NULL_HANDLE ), p_logger( nullptr ) { init_volk( ); }

   context::context( logger* p_logger ) : instance( VK_NULL_HANDLE ), debug_messenger( VK_NULL_HANDLE ), p_logger( p_logger )
   {
      init_volk( );

      uint32 api_version = 0;
      vkEnumerateInstanceVersion( &api_version );

      LOG_INFO_P( p_logger, "Vulkan API version: {1}.{2}.{3}", VK_VERSION_MAJOR( api_version ), VK_VERSION_MINOR( api_version ),
         VK_VERSION_PATCH( api_version ) );

      auto app_info = VkApplicationInfo{};
      app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pNext = nullptr;
      app_info.apiVersion = api_version;
      app_info.engineVersion = VK_MAKE_VERSION( ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH );
      app_info.pEngineName = ENGINE_NAME;
      app_info.applicationVersion = VK_MAKE_VERSION( 0, 0, 0 );
      app_info.pApplicationName = nullptr;

      auto instance_create_info = VkInstanceCreateInfo{};
      instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      instance_create_info.pNext = nullptr;
      instance_create_info.flags = 0;
      instance_create_info.pApplicationInfo = &app_info;

      std::array<char const*, 1> validation_layers = {"VK_LAYER_KHRONOS_validation"};

      if constexpr ( vk::ENABLE_VALIDATION_LAYERS )
      {
         uint32 layer_count = 0;
         vkEnumerateInstanceLayerProperties( &layer_count, nullptr );
         std::vector<VkLayerProperties> layer_properties( layer_count );
         vkEnumerateInstanceLayerProperties( &layer_count, layer_properties.data( ) );

         bool has_khronos_validation = false;
         for ( auto const& property : layer_properties )
         {
            if ( strcmp( validation_layers[0], property.layerName ) == 0 )
            {
               has_khronos_validation = true;
            }
         }

         if ( has_khronos_validation )
         {
            instance_create_info.enabledLayerCount = 1;
            instance_create_info.ppEnabledLayerNames = validation_layers.data( );
         }
         else
         {
            LOG_WARN_P( p_logger, "Layer Property \"{1}\" not found. No Validation layer support will be provided", validation_layers[0] );

            instance_create_info.enabledLayerCount = 0;
            instance_create_info.ppEnabledLayerNames = nullptr;
         }
      }
      else
      {
         instance_create_info.enabledLayerCount = 0;
         instance_create_info.ppEnabledLayerNames = nullptr;
      }

      std::vector<char const*> instance_extensions;

      // Get the instance extensions
      std::uint32_t instance_extension_count = 0;
      vkEnumerateInstanceExtensionProperties( nullptr, &instance_extension_count, nullptr );
      std::vector<VkExtensionProperties> instance_extensions_properties( instance_extension_count );
      vkEnumerateInstanceExtensionProperties( nullptr, &instance_extension_count, instance_extensions_properties.data( ) );

      instance_extensions.reserve( instance_extension_count );

      bool has_surface_extension = false;
      bool has_xcb_extension = false;
      for ( auto const& property : instance_extensions_properties )
      {
         if ( strcmp( "VK_KHR_surface", property.extensionName ) == 0 )
         {
            has_surface_extension = true;
         }

         if ( strcmp( "VK_KHR_xcb_surface", property.extensionName ) == 0 )
         {
            has_xcb_extension = true;
         }

         instance_extensions.emplace_back( property.extensionName );

         LOG_INFO_P( p_logger, "Instance extension \"{1}\" enabled", property.extensionName );
      }

      instance_create_info.enabledExtensionCount = instance_extension_count;
      instance_create_info.ppEnabledExtensionNames = instance_extensions.data( );

      auto instance_res = vkCreateInstance( &instance_create_info, nullptr, &instance );
      if ( instance_res != VK_SUCCESS )
      {
         LOG_ERROR_P( p_logger, "Failed to create vulkan instance: {1}", vk::result_to_string( instance_res ) );
      }
      else
      {
         LOG_INFO_P( p_logger, "Vulkan instance created: 0x{1:x}", reinterpret_cast<std::uintptr_t>( &instance ) );
      }

      volkLoadInstance( instance );

      if constexpr ( vk::ENABLE_VALIDATION_LAYERS )
      {
         VkDebugUtilsMessengerCreateInfoEXT create_info = {};
         create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
         create_info.pNext = nullptr;
         create_info.flags = 0;
         create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
         create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
         create_info.pfnUserCallback = debug_callback;
         create_info.pUserData = nullptr;

         auto debug_messenger_res = vkCreateDebugUtilsMessengerEXT( instance, &create_info, nullptr, &debug_messenger );
         if ( debug_messenger_res != VK_SUCCESS )
         {
            LOG_ERROR_P( p_logger, "Failed to create vulkan debug utils messenger: {1}", vk::result_to_string( debug_messenger_res ) );
         }
         else
         {
            LOG_INFO_P( p_logger, "Vulkan debug utils messenger created: 0x{1:x}", reinterpret_cast<std::uintptr_t>( &debug_messenger ) );
         }
      }

      uint32 physical_device_count = 0;
      vkEnumeratePhysicalDevices( instance, &physical_device_count, nullptr );
      available_physical_devices.resize( physical_device_count );
      vkEnumeratePhysicalDevices( instance, &physical_device_count, available_physical_devices.data( ) );

      if ( available_physical_devices.size( ) == 0 )
      {
         LOG_ERROR( p_logger, "Failed to find a usable graphics card for rendering" );
      }
   }

   context::~context( )
   {
      if ( debug_messenger != VK_NULL_HANDLE )
      {
         vkDestroyDebugUtilsMessengerEXT( instance, debug_messenger, nullptr );
         debug_messenger = VK_NULL_HANDLE;
      }

      if ( instance != VK_NULL_HANDLE )
      {
         vkDestroyInstance( instance, nullptr );
         instance = VK_NULL_HANDLE;
      }
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
