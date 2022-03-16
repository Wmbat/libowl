#include <libash/instance.hpp>

#include <libash/core.hpp>
#include <libash/runtime_error.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <spdlog/logger.h>

#include <magic_enum.hpp>

#include <cstring>
#include <optional>
#include <string_view>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE; // NOLINT

static VKAPI_ATTR auto VKAPI_CALL debug_callback(
   VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
   VkDebugUtilsMessageTypeFlagsEXT messageType,
   const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data) -> VkBool32
{
   assert(p_user_data != nullptr); // NOLINT

   auto* p_logger = static_cast<spdlog::logger*>(p_user_data);

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
         p_logger->warn("{0} - {1}", type, p_callback_data->pMessage);
      }
      else
      {
         p_logger->warn("{0}", p_callback_data->pMessage);
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

namespace ash::inline v0
{
   namespace detail
   {
      auto is_layer_available(std::string_view name,
                              std::span<const vk::LayerProperties> available_layers) -> bool
      {
         bool is_layer_found = false;
         for (const auto& layer : available_layers)
         {
            if (std::string_view(layer.layerName) == name)
            {
               is_layer_found = true;
            }
         }

         return is_layer_found;
      }

      auto is_extension_available(std::string_view name,
                                  std::span<const vk::ExtensionProperties> extensions) -> bool
      {
         bool is_extension_found = false;
         for (const auto& ext : extensions)
         {
            if (std::string_view(ext.extensionName) == name)
            {
               is_extension_found = true;
            }
         }

         return is_extension_found;
      }

      auto get_windowing_extensions(std::span<const vk::ExtensionProperties> properties)
         -> std::optional<std::string_view>
      {
         using namespace std::literals;

#if defined(__linux__)
#   if defined(VK_USE_PLATFORM_XCB_KHR)
         if (is_extension_available("VK_KHR_xcb_surface", properties))
         {
            return "VK_KHR_xcb_surface"sv;
         }
#   elif defined(VK_USE_PLATFORM_XLIB_KHR)
         if (is_extension_available("VK_KHR_xlib_surface", properties))
         {
            return VK_KHR_xlib_surface "sv;
         }
#   elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
         if (is_extension_available("VK_KHR_wayland_surface", properties))
         {
            return "VK_KHR_wayland_surface"sv;
         }
#   endif

         return std::nullopt;

#elif defined(_WIN32)
         if (is_extension_available("VK_KHR_win32_surface", properties))
         {
            return "VK_KHR_win32_surface"sv;
         }
         else
         {
            return std::nullopt;
         }
#else
         return std::nullopt;
#endif
      }
   } // namespace detail

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

   auto load_vulkan(spdlog::logger* p_logger) -> vk::DynamicLoader;
   auto create_vulkan_instance(const instance_create_info& info,
                               mannele::semantic_version api_version, spdlog::logger* p_logger)
      -> vk::UniqueInstance;
   auto create_debug_utils(const vk::UniqueInstance& inst, spdlog::logger* p_logger)
      -> vk::UniqueDebugUtilsMessengerEXT;

   void check_for_unsupported_exts(std::span<const char*> ext_names,
                                   std::span<const vk::ExtensionProperties> extensions)
   {
      for (const char* ext_name : ext_names)
      {
         if (!detail::is_extension_available(ext_name, extensions))
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

   instance::instance(instance_create_info&& info) :
      mp_logger(&info.logger), m_loader(load_vulkan(mp_logger)),
      m_api_version(detail::from_vulkan_version(vk::enumerateInstanceVersion())),
      m_instance(create_vulkan_instance(info, m_api_version, mp_logger)),
      m_debug_utils(create_debug_utils(m_instance, mp_logger))
   {}

   instance::operator vk::Instance() const { return m_instance.get(); }

   auto instance::version() const noexcept -> mannele::semantic_version { return m_api_version; }

   auto load_vulkan(spdlog::logger* p_logger) -> vk::DynamicLoader
   {
      vk::DynamicLoader loader{};

      auto vk_get_instance_proc_addr =
         loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
      VULKAN_HPP_DEFAULT_DISPATCHER.init(vk_get_instance_proc_addr);

      p_logger->debug("core vulkan functionalities loaded");

      return loader;
   }
   auto create_vulkan_instance(const instance_create_info& info,
                               mannele::semantic_version api_version, spdlog::logger* p_logger)
      -> vk::UniqueInstance
   {
      std::vector layer_properties = vk::enumerateInstanceLayerProperties();
      std::vector extension_properties = vk::enumerateInstanceExtensionProperties();

      const auto window_ext = detail::get_windowing_extensions(extension_properties);

      if (!(info.is_headless || window_ext))
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
            p_logger->warn("Khronos validation layers not found");
         }

         if (detail::is_extension_available("VK_EXT_debug_utils", extension_properties))
         {
            ext_names.push_back("VK_EXT_debug_utils");
         }
         else
         {
            p_logger->warn("Debug utils not supported by vulkan instance");
         }
      }

      if (window_ext)
      {
         ext_names.push_back(window_ext.value().data());
      }

      check_for_unsupported_exts(ext_names, extension_properties);
      check_for_unsupported_layers(layer_names, layer_properties);

      for (const char* name : ext_names)
      {
         p_logger->debug("enabled extension: {}", name);
      }

      for (const char* name : layer_names)
      {
         p_logger->debug("enabled layer: {}", name);
      }

      const u32 app_version = detail::to_vulkan_version(info.app_info.version);
      const u32 engine_version = detail::to_vulkan_version(info.eng_info.version);

      const auto app_info = vk::ApplicationInfo()
                               .setApiVersion(detail::to_vulkan_version(api_version))
                               .setPApplicationName(std::data(info.app_info.name))
                               .setApplicationVersion(app_version)
                               .setPEngineName(std::data(info.eng_info.name))
                               .setEngineVersion(engine_version);
      const auto create_info = vk::InstanceCreateInfo()
                                  .setPApplicationInfo(&app_info)
                                  .setPEnabledLayerNames(layer_names)
                                  .setPEnabledExtensionNames(ext_names);
      auto instance = vk::createInstanceUnique(create_info);

      VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

      return instance;
   }
   auto create_debug_utils(const vk::UniqueInstance& inst, spdlog::logger* p_logger)
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
            .setPUserData(static_cast<void*>(p_logger));

      if constexpr (enable_validation_layers)
      {
         std::vector extension_properties = vk::enumerateInstanceExtensionProperties();

         if (detail::is_extension_available("VK_EXT_debug_utils", extension_properties))
         {
            return inst->createDebugUtilsMessengerEXTUnique(create_info, nullptr,
                                                            VULKAN_HPP_DEFAULT_DISPATCHER);
         }

         return {};
      }
      else
      {
         return {};
      }
   }

} // namespace ash::inline v0
