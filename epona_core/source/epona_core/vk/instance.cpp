#include "epona_core/vk/instance.hpp"
#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/details/logger.hpp"
#include "vulkan/vulkan_core.h"

#include <cstring>
#include <ranges>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
   VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
   VkDebugUtilsMessageTypeFlagsEXT messageType,
   const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data)
{
   core::logger* p_logger = static_cast<core::logger*>(p_user_data);

   std::string type;
   if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
   {
      type = "GENERAL";
   }
   else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
   {
      type = "VALIDATION";
   }
   else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
   {
      type = "PERFORMANCE";
   }

   if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
   {
      if (!type.empty())
      {
         LOG_ERROR_P(p_logger, "{1} - {2}", type, p_callback_data->pMessage);
      }
      else
      {
         LOG_ERROR_P(p_logger, "{1}", p_callback_data->pMessage);
      }
   }
   else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
   {
      if (!type.empty())
      {
         LOG_WARN_P(p_logger, "{1} - {2}", type, p_callback_data->pMessage);
      }
      else
      {
         LOG_WARN_P(p_logger, "{1}", p_callback_data->pMessage);
      }
   }
   else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
   {
      if (!type.empty())
      {
         LOG_INFO_P(p_logger, "{1} - {2}", type, p_callback_data->pMessage);
      }
      else
      {
         LOG_INFO_P(p_logger, "{1}", p_callback_data->pMessage);
      }
   }

   return VK_FALSE;
}

namespace core::vk
{
   instance::instance(instance&& other) noexcept { *this = std::move(other); }
   instance::~instance()
   {
      if (vk_debug_messenger && vk_instance)
      {
         vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, nullptr);
         vkDestroyInstance(vk_instance, nullptr);
      }
   }

   instance& instance::operator=(instance&& rhs) noexcept
   {
      if (this != &rhs)
      {
         if (vk_instance && vk_debug_messenger)
         {
            vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, nullptr);
            vkDestroyInstance(vk_instance, nullptr);
         }

         vk_instance = rhs.vk_instance;
         rhs.vk_instance = VK_NULL_HANDLE;

         vk_debug_messenger = rhs.vk_debug_messenger;
         rhs.vk_debug_messenger = VK_NULL_HANDLE;

         version = rhs.version;
      }

      return *this;
   }

   struct instance_error_category : std::error_category
   {
      const char* name() const noexcept override { return "vk_instance"; }
      std::string message(int err) const override
      {
         return instance::to_string(static_cast<instance::error>(err));
      }
   };

   const instance_error_category inst_err_cat;

   std::string instance::to_string(error err)
   {
      switch (err)
      {
         case error::vulkan_version_unavailable:
            return "vulkan_version_unavailable";
         case error::vulkan_version_1_2_unavailable:
            return "vulkan_version_1_2_unavailable";
         case error::window_extensions_not_present:
            return "window_extensions_not_present";
         default:
            return "UNKNOWN";
      }
   };

   std::error_code instance::make_error_code(error err)
   {
      return {static_cast<int>(err), inst_err_cat};
   }

   vk::details::result<instance> instance_builder::build(const runtime& rt, logger* const p_logger)
   {
      if (rt.api_version == 0)
      {
         return instance::make_error_code(instance::error::vulkan_version_unavailable);
      }

      if (VK_VERSION_MINOR(rt.api_version) < 2)
      {
         return instance::make_error_code(instance::error::vulkan_version_1_2_unavailable);
      }

      VkApplicationInfo app_info = {};
      app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pNext = nullptr;
      app_info.pApplicationName = info.app_name.c_str();
      app_info.applicationVersion = info.app_version;
      app_info.pEngineName = info.engine_name.c_str();
      app_info.engineVersion = info.engine_version;
      app_info.apiVersion = rt.api_version;

      tiny_dynamic_array<const char*, 16> extensions;
      std::ranges::for_each(info.extensions, [&extensions](const char* str) {
         extensions.push_back(str);
      });

      if constexpr (vk::details::ENABLE_VALIDATION_LAYERS)
      {
         if (rt.debug_utils_available)
         {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
         }
      }

      const auto check_ext_and_add = [&](const char* name) -> bool {
         auto it = std::ranges::find_if(rt.extensions, [&name](const VkExtensionProperties& ext) {
            return strcmp(name, ext.extensionName) == 0;
         });

         if (it != rt.extensions.cend())
         {
            extensions.push_back(name);

            return true;
         }

         return false;
      };

      const bool has_khr_surface_ext = check_ext_and_add("VK_KHR_surface");

#if defined(__linux__)
      // clang-format off
      const bool has_wnd_exts = 
         check_ext_and_add("VK_KHR_xcb_surface") ||
         check_ext_and_add("VK_KHR_xlib_surface") ||
         check_ext_and_add("VK_KHR_wayland_surface");
      // clang-format on
#elif defined(_WIN32)
      const bool has_wnd_exts = check_ext_and_add("VK_KHR_win32_surface");
#else
      const bool has_wnd_exts = false;
#endif

      if (!has_wnd_exts || !has_khr_surface_ext)
      {
         return instance::make_error_code(instance::error::window_extensions_not_present);
      }

      for (const auto& desired : extensions)
      {
         bool is_present = false;
         for (const auto& available : rt.extensions)
         {
            if (strcmp(desired, available.extensionName) == 0)
            {
               is_present = true;
            }
         }

         if (!is_present)
         {
            return instance::make_error_code(instance::error::instance_extension_not_supported);
         }
      }

      std::ranges::for_each(extensions, [p_logger](const char* name) {
         LOG_INFO_P(p_logger, "Instance extension: {1} - ENABLED", name)
      });

      VkInstanceCreateInfo instance_create_info = {};
      instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      instance_create_info.pNext = nullptr;
      instance_create_info.flags = {};
      instance_create_info.pApplicationInfo = &app_info;
      instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
      instance_create_info.ppEnabledExtensionNames = extensions.data();
      instance_create_info.enabledLayerCount = 0;
      instance_create_info.ppEnabledLayerNames = nullptr;

      if constexpr (details::ENABLE_VALIDATION_LAYERS)
      {
         tiny_dynamic_array<const char*, 8> layers;
         std::ranges::for_each(info.layers, [&layers](const char* str) {
            layers.push_back(str);
         });

         if (rt.validation_layers_available)
         {
            layers.push_back("VK_LAYER_KHRONOS_validation");

            for (const auto& desired : layers)
            {
               bool is_present = false;
               for (const auto& available : rt.layers)
               {
                  if (strcmp(desired, available.layerName) == 0)
                  {
                     is_present = true;
                  }
               }

               if (!is_present)
               {
                  return instance::make_error_code(instance::error::instance_layer_not_supported);
               }
            }

            instance_create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
            instance_create_info.ppEnabledLayerNames = layers.data();
         }

         std::ranges::for_each(layers, [p_logger](const char* name) {
            LOG_INFO_P(p_logger, "Instance layers: {1} - ENABLED", name)
         });
      }

      instance inst{};
      inst.version = rt.api_version;

      const VkResult inst_res = vkCreateInstance(&instance_create_info, nullptr, &inst.vk_instance);
      if (inst_res != VK_SUCCESS)
      {
         return details::result<instance>{
            instance::make_error_code(instance::error::failed_create_instance), inst_res};
      }

      volkLoadInstance(inst.vk_instance);

      // clang-format off
      const VkDebugUtilsMessengerCreateInfoEXT debug_create_info
      {
         .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
         .pNext = nullptr,
         .flags = {},
         .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
         .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
         .pfnUserCallback = debug_callback,
         .pUserData = static_cast<void*>(p_logger)
      };
      // clang-format on
      const VkResult debug_res = vkCreateDebugUtilsMessengerEXT(
         inst.vk_instance, &debug_create_info, nullptr, &inst.vk_debug_messenger);
      if (debug_res != VK_SUCCESS)
      {
         return details::result<instance>{
            instance::make_error_code(instance::error::failed_create_debug_utils), debug_res};
      }

      return details::result{std::move(inst)};
   }

   instance_builder& instance_builder::set_application_name(std::string_view app_name)
   {
      info.app_name = app_name;
      return *this;
   }
   instance_builder& instance_builder::set_engine_name(std::string_view engine_name)
   {
      info.engine_name = engine_name;
      return *this;
   }
   instance_builder& instance_builder::set_application_version(
      uint32_t major, uint32_t minor, uint32_t patch)
   {
      info.app_version = VK_MAKE_VERSION(major, minor, patch);
      return *this;
   }
   instance_builder& instance_builder::set_engine_version(
      uint32_t major, uint32_t minor, uint32_t patch)
   {
      info.engine_version = VK_MAKE_VERSION(major, minor, patch);
      return *this;
   }
   instance_builder& instance_builder::enable_layer(const std::string& layer_name)
   {
      if (!layer_name.empty())
      {
         info.layers.push_back(layer_name.c_str());
      }
      return *this;
   }
   instance_builder& instance_builder::enable_extension(const std::string& extension_name)
   {
      if (!extension_name.empty())
      {
         info.extensions.push_back(extension_name.c_str());
      }
      return *this;
   }

}; // namespace core::vk
