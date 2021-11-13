#include <libash/instance.hpp>

#include <libash/core.hpp>
#include <libash/runtime_error.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <magic_enum.hpp>

#include <cstring>
#include <string_view>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE; // NOLINT

static VKAPI_ATTR auto VKAPI_CALL debug_callback(
   VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
   VkDebugUtilsMessageTypeFlagsEXT messageType,
   const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data) -> VkBool32
{
   assert(p_user_data != nullptr); // NOLINT

   auto* p_logger = static_cast<mannele::logger*>(p_user_data);

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
         p_logger->error("{0} - {1}", type, p_callback_data->pMessage);
      }
      else
      {
         p_logger->error("{0}", p_callback_data->pMessage);
      }
   }
   else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
   {
      if (!type.empty())
      {
         p_logger->warning("{0} - {1}", type, p_callback_data->pMessage);
      }
      else
      {
         p_logger->warning("{0}", p_callback_data->pMessage);
      }
   }
   else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
   {
      /*
      if (!type.empty())
      {
         LOG_INFO_P(p_logger, "{1} - {2}", type, p_callback_data->pMessage);
      }
      else
      {
         LOG_INFO_P(p_logger, "{1}", p_callback_data->pMessage);
      }
      */
   }

   return VK_FALSE;
}

namespace ash::detail
{
   auto is_layer_available(std::string_view name,
                            std::span<const vk::LayerProperties> available_layers) -> bool
   {
      for (const auto& layer : available_layers)
      {
         return std::string_view(layer.layerName) == name;
      }

      return false;
   }

   auto is_extension_available(std::span<const vk::ExtensionProperties> extensions,
                                std::string_view name) -> bool
   {
      for (const auto& ext : extensions)
      {
         return std::string_view(ext.extensionName) == name;
      }

      return false;
   }

   auto has_windowing_extensions(std::span<const vk::ExtensionProperties> properties) -> bool
   {
#if defined(__linux__)
      return is_extension_available(properties, "VK_KHR_xcb_surface")
             || is_extension_available(properties, "VK_KHR_xlib_surface")
             || is_extension_available(properties, "VK_KHR_wayland_surface");
#elif defined(_WIN32)
      return check_extension_support(properties, "VK_KHR_win32_surface");
#else
      return false;
#endif
   }
} // namespace ash::detail

namespace ash
{
   struct instance_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override { return "ash::instance"; }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return std::string(magic_enum::enum_name(static_cast<instance_error>(err)));
      }
   };

   auto to_error_condition(instance_error err) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(err), instance_error_category()});
   }

   auto load_vulkan(mannele::log_ptr logger) -> vk::DynamicLoader;
   auto create_vulkan_instance(const instance_create_info& info, u32 api_version,
                               mannele::log_ptr logger) -> vk::UniqueInstance;
   auto create_debug_utils(const vk::UniqueInstance& inst, mannele::log_ptr logger)
      -> vk::UniqueDebugUtilsMessengerEXT;

   void check_for_unsupported_exts(std::span<const char*> ext_names,
                                   std::span<const vk::ExtensionProperties> extensions)
   {
      for (const char* ext_name : ext_names)
      {
         if (!detail::is_extension_available(extensions, ext_name))
         {
            throw runtime_error(to_error_condition(instance_error::extension_support_not_found));
         }
      }
   }

   void check_for_unsupported_layers(std::span<const char*> layer_names,
                                     std::span<const vk::LayerProperties> layers)
   {
      for (const char* layer_name : layer_names)
      {
         if (!detail::is_layer_available(layer_name, layers))
         {
            throw runtime_error(to_error_condition(instance_error::layer_support_not_found));
         }
      }
   }

   auto format_char_array(std::span<const char*> arr)
      -> fmt::basic_runtime<fmt::char_t<std::basic_string<char>>>
   {
      return fmt::runtime(fmt::format(fmt::runtime("Enabled Extensions: {}"), arr));
   }

   instance::instance(const instance_create_info& info) :
      m_loader(load_vulkan(info.logger)), m_api_version(vk::enumerateInstanceVersion()),
      m_instance(create_vulkan_instance(info, m_api_version, info.logger)),
      m_debug_utils(create_debug_utils(m_instance, info.logger))
   {}

   instance::operator vk::Instance() { return m_instance.get(); }

   auto instance::version() const noexcept -> u32 { return m_api_version; }

   auto load_vulkan(mannele::log_ptr logger) -> vk::DynamicLoader
   {
      vk::DynamicLoader loader{};

      auto vk_get_instance_proc_addr =
         loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
      VULKAN_HPP_DEFAULT_DISPATCHER.init(vk_get_instance_proc_addr);

      logger.debug("core vulkan functionalities loaded");

      return loader;
   }
   auto create_vulkan_instance(const instance_create_info& info, u32 api_version,
                               mannele::log_ptr logger) -> vk::UniqueInstance
   {
      std::vector layer_properties = vk::enumerateInstanceLayerProperties();
      std::vector extension_properties = vk::enumerateInstanceExtensionProperties();

      if (!info.is_headless and !detail::has_windowing_extensions(extension_properties))
      {
         throw runtime_error(to_error_condition(instance_error::window_support_not_found));
      }

      std::vector ext_names = info.enabled_extension_names;
      std::vector layer_names = info.enabled_layer_names;

      if constexpr (enable_validation_layers)
      {
         if (detail::is_layer_available("VK_LAYER_KHRONOS_validation", layer_properties))
         {
            layer_names.push_back("VK_LAYER_KHRONOS_validation");
         }
         else
         {
            logger.warning("Khronos validation layers not found");
         }
      }

      check_for_unsupported_exts(ext_names, extension_properties);
      check_for_unsupported_layers(layer_names, layer_properties);

      logger.debug(format_char_array(ext_names));
      logger.debug(format_char_array(layer_names));

      const u32 app_version = detail::to_vulkan_version(info.app_info.version);
      const u32 engine_version = detail::to_vulkan_version(info.engine_info.version);

      const auto app_info = vk::ApplicationInfo()
                               .setApiVersion(api_version)
                               .setPApplicationName(std::data(info.app_info.name))
                               .setApplicationVersion(app_version)
                               .setPEngineName(std::data(info.engine_info.name))
                               .setEngineVersion(engine_version);
      const auto create_info = vk::InstanceCreateInfo()
                                  .setPApplicationInfo(&app_info)
                                  .setPEnabledLayerNames(layer_names)
                                  .setPEnabledExtensionNames(ext_names);

      return vk::createInstanceUnique(create_info);
   }
   auto create_debug_utils(const vk::UniqueInstance& inst, mannele::log_ptr logger)
      -> vk::UniqueDebugUtilsMessengerEXT
   {
      const auto create_info =
         vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            .setPfnUserCallback(debug_callback)
            .setPUserData(static_cast<void*>(logger.get()));

      return inst->createDebugUtilsMessengerEXTUnique(create_info, nullptr,
                                                      VULKAN_HPP_DEFAULT_DISPATCHER);
   }

} // namespace ash
