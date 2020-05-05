/**
 * MIT License
 *
 * Copyright (c) 2020 Wmbat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <epona_core/vk/runtime.hpp>
#include <epona_library/utils/logger.hpp>

#include <alloca.h>

namespace EGL::vk
{
   static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
      void* p_user_data )
   {
      if ( message_type != 0 )
      {
      }

      if ( p_user_data != nullptr )
      {
         auto* p_logger = reinterpret_cast<ESL::logger*>( p_user_data );

         if ( message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
         {
            LOG_INFO_P( p_logger, "{1}", p_callback_data->pMessage );
         }
         else if ( message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
         {
            LOG_WARN_P( p_logger, "{1}", p_callback_data->pMessage );
         }
         else if ( message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
         {
            LOG_ERROR_P( p_logger, "{1}", p_callback_data->pMessage );
         }
      }
      else
      {
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
      }

      return VK_FALSE;
   }

   runtime::runtime( ESL::multipool_allocator* p_main_allocator ) : p_main_allocator( p_main_allocator )
   {
      api_version = volkGetInstanceVersion( );
   }
   runtime::runtime( std::string_view app_name_in, ESL::multipool_allocator* p_main_allocator, ESL::logger* p_log ) :
      p_log( p_log ), app_name( app_name_in ), p_main_allocator( p_main_allocator )
   {
      api_version = volkGetInstanceVersion( );

      LOG_INFO_P( p_log, "Vulkan API version: {1}.{2}.{3}", VK_VERSION_MAJOR( api_version ),
         VK_VERSION_MINOR( api_version ), VK_VERSION_PATCH( api_version ) );
   }
   runtime::runtime( runtime&& other ) : p_main_allocator( other.p_main_allocator )
   {
      p_log = other.p_log;

      other.p_main_allocator = nullptr;

      instance_extensions = std::move( other.instance_extensions );
      app_name = std::move( other.app_name );

      instance = other.instance;
      other.instance = VK_NULL_HANDLE;

      debug_messenger = other.debug_messenger;
      other.debug_messenger = VK_NULL_HANDLE;
   }
   runtime::~runtime( )
   {
      if constexpr ( ENABLE_VALIDATION_LAYERS )
      {
         if ( debug_messenger != VK_NULL_HANDLE )
         {
            vkDestroyDebugUtilsMessengerEXT( instance, debug_messenger, nullptr );
         }
      }

      if ( instance != VK_NULL_HANDLE )
      {
         vkDestroyInstance( instance, nullptr );
      }
   }

   runtime& runtime::operator=( runtime&& rhs )
   {
      if ( this != &rhs )
      {
         p_log = rhs.p_log;

         p_main_allocator = rhs.p_main_allocator;
         rhs.p_main_allocator = nullptr;

         app_name = std::move( rhs.app_name );
         instance_extensions = std::move( rhs.instance_extensions );

         instance = rhs.instance;
         rhs.instance = VK_NULL_HANDLE;

         debug_messenger = rhs.debug_messenger;
         rhs.debug_messenger = VK_NULL_HANDLE;
      }

      return *this;
   }

   runtime&& runtime::create_instance( )
   {
      VkApplicationInfo app_info = { };
      app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pNext = nullptr;
      app_info.pApplicationName = app_name.c_str( );
      app_info.applicationVersion = VK_MAKE_VERSION( 0, 0, 0 );
      app_info.pEngineName = "EGL";
      app_info.engineVersion = VK_MAKE_VERSION( CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH );
      app_info.apiVersion = api_version;

      std::uint32_t glfw_ext_count = 0;
      char const** glfw_ext = glfwGetRequiredInstanceExtensions( &glfw_ext_count );

      instance_extensions.assign( glfw_ext, glfw_ext + glfw_ext_count );
      instance_extensions.emplace_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

      VkInstanceCreateInfo instance_create_info = { };
      instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      instance_create_info.pNext = nullptr;
      instance_create_info.flags = { };
      instance_create_info.pApplicationInfo = &app_info;
      instance_create_info.enabledExtensionCount = instance_extensions.size( );
      instance_create_info.ppEnabledExtensionNames = instance_extensions.data( );

      if ( check_validation_layer_support( ) )
      {
         instance_create_info.enabledLayerCount = validation_layer.size( );
         instance_create_info.ppEnabledLayerNames = validation_layer.data( );
      }
      else
      {
         instance_create_info.enabledLayerCount = 0;
         instance_create_info.ppEnabledLayerNames = nullptr;
      }

      if ( vkCreateInstance( &instance_create_info, nullptr, &instance ) != VK_SUCCESS )
      {
         throw;
      }

      volkLoadInstance( instance );

      if constexpr ( ENABLE_VALIDATION_LAYERS )
      {
         VkDebugUtilsMessengerCreateInfoEXT create_info = { };
         create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
         create_info.pNext = nullptr;
         create_info.flags = { };
         create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
         create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
         create_info.pfnUserCallback = debug_callback;
         create_info.pUserData = p_log;

         if ( vkCreateDebugUtilsMessengerEXT( instance, &create_info, nullptr, &debug_messenger ) != VK_SUCCESS )
         {
            LOG_ERROR( p_log, "Failed to create debug utils messenger. No validation layers support will be enabled." );
         }
      }

      return std::move( *this );
   }

   bool runtime::check_validation_layer_support( )
   {
      if constexpr ( ENABLE_VALIDATION_LAYERS )
      {
         std::uint32_t layer_count = 0;
         vkEnumerateInstanceLayerProperties( &layer_count, nullptr );
         auto* p_layers = reinterpret_cast<VkLayerProperties*>( alloca( sizeof( VkLayerProperties ) * layer_count ) );
         vkEnumerateInstanceLayerProperties( &layer_count, p_layers );

         for ( auto const& layer : validation_layer )
         {
            for ( std::size_t i = 0; i < layer_count; ++i )
            {
               if ( strcmp( layer, p_layers[i].layerName ) == 0 )
               {
                  LOG_INFO_P( p_log, "Validation layer support: {1}", layer );

                  return true;
               }
            }
         }

         return false;
      }
      else
      {
         return false;
      }
   }

   ESL::vector<char const*, ESL::multipool_allocator> runtime::get_instance_extensions( )
   {
      ESL::vector<char const*, ESL::multipool_allocator> ext( p_main_allocator );

      std::uint32_t count = 0;
      vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
      auto* p_ext_prop = reinterpret_cast<VkExtensionProperties*>( alloca( sizeof( VkExtensionProperties ) * count ) );
      vkEnumerateInstanceExtensionProperties( nullptr, &count, p_ext_prop );

      // TODO: don't enable everything by default.

      ext.reserve( count );
      for ( std::size_t i = 0; i < count; ++i )
      {
         ext.emplace_back( p_ext_prop[i].extensionName );
      }

      return ext;
   }

} // namespace EGL::vk
