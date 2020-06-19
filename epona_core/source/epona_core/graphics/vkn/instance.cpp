#include "epona_core/graphics/vkn/instance.hpp"
#include "epona_core/detail/monad/either.hpp"

#include <functional>

namespace core::gfx::vkn
{
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

   namespace detail
   {
      std::string to_string(instance::error err)
      {
         using error = instance::error;

         switch (err)
         {
            case error::vulkan_version_unavailable:
               return "vulkan_version_unavailable";
            case error::vulkan_version_1_2_unavailable:
               return "vulkan_version_1_2_unavailable";
            case error::window_extensions_not_present:
               return "window_extensions_not_present";
            case error::instance_extension_not_supported:
               return "instance_extension_not_supported";
            case error::instance_layer_not_supported:
               return "instance_layer_not_supported";
            case error::failed_to_create_instance:
               return "failed_create_instance";
            case error::failed_to_create_debug_utils:
               return "failed_create_debug_utils";
            default:
               return "UNKNOWN";
         }
      };

      struct instance_error_category : std::error_category
      {
         const char* name() const noexcept override { return "vk_instance"; }
         std::string message(int err) const override
         {
            return to_string(static_cast<instance::error>(err));
         }
      };

      const instance_error_category inst_err_cat;

      std::error_code make_error_code(instance::error err)
      {
         return {static_cast<int>(err), inst_err_cat};
      }
   } // namespace detail

   instance_builder::instance_builder(const loader& vk_loader, logger* const p_logger) :
      vk_loader{vk_loader}, p_logger{p_logger}
   {}

   result<instance> instance_builder::build()
   {
      auto version_res = monad::try_wrap<vk::SystemError>([&] {
         return vk::enumerateInstanceVersion(); // may throw
      });

      if (version_res.is_left())
      {
         return monad::left<error>{.val = *(version_res.left() >>= [](const vk::SystemError& e) {
            return error{
               .type = detail::make_error_code(instance::error::vulkan_version_unavailable),
               .result = static_cast<vk::Result>(e.code().value())};
         })};
      }

      // clang-format off
      const auto available_layers = monad::try_wrap<vk::SystemError>([&] {
            return vk::enumerateInstanceLayerProperties();
         })
         .join(
            [&]([[maybe_unused]] const vk::SystemError& e) {
               LOG_ERROR_P(p_logger, "Instance layer enumeration error: {1}", e.what());
               return dynamic_array<vk::LayerProperties>();  // handle error.
            },
            [](const std::vector<vk::LayerProperties>& data) {
               return dynamic_array<vk::LayerProperties>(data.begin(), data.end());
         });

      const auto available_exts = monad::try_wrap<vk::SystemError>([&] {
            return vk::enumerateInstanceExtensionProperties();
         })
         .join(
            [&]([[maybe_unused]] const vk::SystemError& e) {
               LOG_ERROR_P(p_logger, "Instance ext enumeration error: {1}", e.what());
               return dynamic_array<vk::ExtensionProperties>();  // handle error.
            },
            [](const std::vector<vk::ExtensionProperties>& data) {
               return dynamic_array<vk::ExtensionProperties>(data.begin(), data.end());
         });
      // clang-format on

      const uint32_t api_version = version_res.right().value();
      const bool validation_layers_available = has_validation_layer_support(available_layers);
      const bool debug_utils_available = has_debug_utils_support(available_exts);

      if (VK_VERSION_MINOR(api_version) < 2)
      {
         // clang-format off
         return monad::left<error>{.val = {
            .type = detail::make_error_code(instance::error::vulkan_version_1_2_unavailable),
            .result = {}
         }};
         // clang-format on
      }

      // clang-format off
      const auto app_info = vk::ApplicationInfo{}
         .setPNext(nullptr)
         .setPApplicationName(info.app_name.c_str())
         .setApplicationVersion(info.app_version)
         .setPEngineName(info.engine_name.c_str())
         .setEngineVersion(info.engine_version)
         .setApiVersion(api_version);
      // clang-format on

      auto extension_names_res = get_all_ext(available_exts, debug_utils_available);
      if (const auto left = extension_names_res.left())
      {
         return monad::left<error>{.val = left.value()};
      }

      auto extensions = std::move(extension_names_res.right().value());
      std::ranges::for_each(extensions, [&](const char* name) {
         LOG_INFO_P(p_logger, "vk - instance extension: {1} - ENABLED", name)
      });

      // clang-format off
      auto instance_create_info = vk::InstanceCreateInfo{}
         .setPNext(nullptr)
         .setFlags({})
         .setPApplicationInfo(&app_info)
         .setEnabledLayerCount(0)
         .setPpEnabledLayerNames(nullptr)
         .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
         .setPpEnabledExtensionNames(extensions.data());
      // clang-format on

      if constexpr (detail::ENABLE_VALIDATION_LAYERS)
      {
         tiny_dynamic_array<const char*, 8> layers;
         std::ranges::for_each(info.layers, [&layers](const char* str) {
            layers.push_back(str);
         });

         if (validation_layers_available)
         {
            layers.push_back("VK_LAYER_KHRONOS_validation");

            for (const auto& desired : layers)
            {
               bool is_present = false;
               for (const auto& available : available_layers)
               {
                  if (strcmp(desired, available.layerName) == 0)
                  {
                     is_present = true;
                  }
               }

               if (!is_present)
               {
                  // clang-format off
                  return monad::left<error>{.val = {
                     .type = detail::make_error_code(instance::error::instance_layer_not_supported),
                     .result = {}
                  }};
                  // clang-format on
               }
            }

            instance_create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
            instance_create_info.ppEnabledLayerNames = layers.data();
         }

         std::ranges::for_each(layers, [&](const char* name) {
            LOG_INFO_P(p_logger, "vk - instance layers: {1} - ENABLED", name)
         });
      }

      auto instance_res = monad::try_wrap<vk::SystemError>([&] {
         return vk::createInstanceUnique(instance_create_info); // may throw
      });

      if (instance_res.is_left())
      {
         return monad::left<error>{.val = *(instance_res.left() >>= [](const vk::SystemError& e) {
            return error{
               .type = detail::make_error_code(instance::error::failed_to_create_instance),
               .result = static_cast<vk::Result>(e.code().value())};
         })};
      }

      instance data;
      data.inst = *instance_res.right();
      data.extensions = extensions;
      data.version = api_version;

      LOG_INFO(p_logger, "vk - instance created");

      vk_loader.load_instance(data.inst.get());

      if constexpr (detail::ENABLE_VALIDATION_LAYERS)
      {
         // clang-format off
         const auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT{}
            .setPNext(nullptr)
            .setFlags({})
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
               vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
               vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            .setPfnUserCallback(debug_callback)
            .setPUserData(static_cast<void*>(p_logger));
         // clang-format on

         auto debug_res = monad::try_wrap<vk::SystemError>([&] {
            return data.inst->createDebugUtilsMessengerEXTUnique(debug_create_info);
         });

         if (debug_res.is_left())
         {
            return monad::left<error>{
               .val = *(debug_res.left() >>= [](const vk::SystemError& e) -> error {
                  return error{
                     .type = detail::make_error_code(instance::error::failed_to_create_debug_utils),
                     .result = static_cast<vk::Result>(e.code().value())};
               })};
         }

         LOG_INFO(p_logger, "vk - debug utils created");

         data.debug_utils = *debug_res.right();
      }

      return monad::right<instance>{std::move(data)};
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

   bool instance_builder::has_validation_layer_support(
      const dynamic_array<vk::LayerProperties>& properties) const
   {
      return std::ranges::find_if(properties, [](const VkLayerProperties& layer) {
         return strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0;
      }) != properties.end();
   }

   bool instance_builder::has_debug_utils_support(
      const dynamic_array<vk::ExtensionProperties>& properties) const
   {
      return std::ranges::find_if(properties, [](const VkExtensionProperties& ext) {
         return strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
      }) != properties.end();
   }

   result<tiny_dynamic_array<const char*, 16>> instance_builder::get_all_ext(
      const dynamic_array<vk::ExtensionProperties>& properties,
      bool are_debug_utils_available) const
   {
      tiny_dynamic_array<const char*, 16> extensions;
      std::ranges::for_each(info.extensions, [&extensions](const char* str) {
         extensions.push_back(str);
      });

      if constexpr (detail::ENABLE_VALIDATION_LAYERS)
      {
         if (are_debug_utils_available)
         {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
         }
      }

      const auto check_ext_and_add = [&](const char* name) -> bool {
         auto it = std::ranges::find_if(properties, [&name](const VkExtensionProperties& ext) {
            return strcmp(name, ext.extensionName) == 0;
         });

         if (it != properties.cend())
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
         // clang-format off
         return monad::left<error>{.val = {
            .type = detail::make_error_code(instance::error::window_extensions_not_present),
            .result = {}
         }};
         // clang-format on
      }

      for (const auto& desired : extensions)
      {
         bool is_present = false;
         for (const auto& available : properties)
         {
            if (strcmp(desired, available.extensionName) == 0)
            {
               is_present = true;
            }
         }

         if (!is_present)
         {
            // clang-format off
            return monad::left<error>{.val = {
               .type = detail::make_error_code(instance::error::instance_extension_not_supported),
               .result = {}
            }};
            // clang-format on
         }
      }

      return monad::right<decltype(extensions)>{.val = extensions};
   }
} // namespace core::gfx::vkn
