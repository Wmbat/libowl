#include <cacao/context.hpp>

#include <cacao/error.hpp>
#include <cacao/util/error.hpp>

#include <range/v3/range/conversion.hpp>

#include <ranges>
#include <span>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE; // NOLINT

static VKAPI_ATTR auto VKAPI_CALL debug_callback(
   VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
   VkDebugUtilsMessageTypeFlagsEXT messageType,
   const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data) -> VkBool32
{
   EXPECT(p_user_data != nullptr);

   auto* p_logger = static_cast<util::logger*>(p_user_data);

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

auto has_windowing_extensions(std::span<const vk::ExtensionProperties> properties) -> bool
{
#if defined(__linux__)
   return cacao::check_extension_support(properties, "VK_KHR_xcb_surface") ||
      cacao::check_extension_support(properties, "VK_KHR_xlib_surface") ||
      cacao::check_extension_support(properties, "VK_KHR_wayland_surface");
#elif defined(_WIN32)
   return cacao::check_extension_support(properties, "VK_KHR_win32_surface");
#else
   return false;
#endif
}

namespace cacao
{
   context::context(context_create_info&& info) :
      m_loader{load_vulkan_core(info.logger)},
      m_api_version{get_vulkan_api_version(info.min_vulkan_version)},
      m_instance{create_instance(info)},
      m_debug_utils{create_debug_utils(info.logger)},
      m_logger{info.logger}
   {
      m_logger.info("[cacao] general context created.");
      m_logger.info("[cacao] using vulkan version: {}.{}.{}.", VK_VERSION_MAJOR(m_api_version),
                    VK_VERSION_MINOR(m_api_version), VK_VERSION_PATCH(m_api_version));
   }

   auto context::instance() const noexcept -> vk::Instance { return m_instance.get(); }
   auto context::vulkan_version() const noexcept -> std::uint32_t { return m_api_version; }

   auto context::enumerate_physical_devices() const
      -> crl::small_dynamic_array<vk::PhysicalDevice, 2>
   {
      const std::vector physical_devices = m_instance->enumeratePhysicalDevices();

      return {std::begin(physical_devices), std::end(physical_devices)};
   }

   auto context::load_vulkan_core(util::logger_wrapper logger) const -> vk::DynamicLoader
   {
      vk::DynamicLoader loader{};

      VULKAN_HPP_DEFAULT_DISPATCHER.init(
         loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

      logger.info("[cacao] core functionalities loaded");

      return loader;
   }
   auto context::get_vulkan_api_version(std::uint32_t minimum_version) const -> std::uint32_t
   {
      std::uint32_t vulkan_api_version = vk::enumerateInstanceVersion();

      if (VK_VERSION_MAJOR(vulkan_api_version) < VK_VERSION_MAJOR(minimum_version))
      {
         throw runtime_error{to_error_cond(error_code::minumum_vulkan_version_not_met)};
      }

      if (VK_VERSION_MINOR(vulkan_api_version) < VK_VERSION_MINOR(minimum_version))
      {
         throw runtime_error{to_error_cond(error_code::minumum_vulkan_version_not_met)};
      }

      return vulkan_api_version;
   }
   auto context::create_instance(const context_create_info& info) const -> vk::UniqueInstance
   {
      util::logger_wrapper logger = info.logger;

      std::vector layer_properties = vk::enumerateInstanceLayerProperties();
      std::vector extension_properties = vk::enumerateInstanceExtensionProperties();

      if (info.use_window && !has_windowing_extensions(extension_properties))
      {
         throw runtime_error{to_error_cond(error_code::window_support_requested_but_not_found)};
      }

      std::vector<const char*> layer_names;
      if constexpr (detail::enable_validation_layers)
      {
         if (check_layer_support(layer_properties, "VK_LAYER_KHRONOS_validation"))
         {
            layer_names.push_back("VK_LAYER_KHRONOS_validation");
         }
         else
         {
            logger.warning("Khronos validation layers not found");
         }
      }

      std::vector<const char*> ext_names;
      ext_names.reserve(std::size(extension_properties));

      for (auto& extension : extension_properties)
      {
         ext_names.push_back(extension.extensionName);
      }

      for (const char* name : ext_names)
      {
         logger.info("[cacao] instance extension: {}", name);
      }

      for (const char* name : layer_names)
      {
         logger.info("[cacao] instance layer: {}", name);
      }

      const auto app_info = vk::ApplicationInfo{.apiVersion = m_api_version};

      auto instance = vk::createInstanceUnique(vk::InstanceCreateInfo{}
                                                  .setPApplicationInfo(&app_info)
                                                  .setPEnabledLayerNames(layer_names)
                                                  .setPEnabledExtensionNames(ext_names));

      VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

      return instance;
   }
   auto context::create_debug_utils(util::logger_wrapper logger) const
      -> vk::UniqueDebugUtilsMessengerEXT
   {
      return m_instance->createDebugUtilsMessengerEXTUnique(
         vk::DebugUtilsMessengerCreateInfoEXT{}
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            .setPfnUserCallback(debug_callback)
            .setPUserData(static_cast<void*>(logger.get())));
   }

   auto check_layer_support(std::span<const vk::LayerProperties> layers, std::string_view name)
      -> bool
   {
      return std::ranges::find_if(layers, [name](const vk::LayerProperties& layer) {
                return strcmp(static_cast<const char*>(layer.layerName), name.data()) == 0;
             }) != std::end(layers);
   }

   auto check_extension_support(std::span<const vk::ExtensionProperties> extensions,
                                std::string_view name) -> bool
   {
      return std::ranges::find_if(extensions, [name](const auto& ext) {
                return strcmp(name.data(), static_cast<const char*>(ext.extensionName)) == 0;
             }) != std::end(extensions);
   }
} // namespace cacao
