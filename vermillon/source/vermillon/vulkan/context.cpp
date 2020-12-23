#include <vermillon/vulkan/context.hpp>

#include <monads/try.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

#include <ranges>
#include <span>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace vkn
{
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

   struct context_data
   {
      vk::DynamicLoader dynamic_loader{};

      vk::UniqueInstance instance{nullptr};
      vk::UniqueDebugUtilsMessengerEXT debug_utils{nullptr};

      std::uint32_t api_version{};

      crl::dynamic_array<vk::ExtensionProperties> enabled_extensions{};

      util::logger_wrapper logger{nullptr};
   };

   /**
    * FORWARD DECLARATIONS
    */

   auto load_core(util::logger_wrapper logger) -> context_data;
   auto query_vulkan_version(context_data&& data) -> util::result<context_data>;
   auto create_instance(context_data&& data) -> util::result<context_data>;
   auto create_debug_utils(context_data&& data) -> util::result<context_data>;

   /**
    * MAIN CONSTRUCTION FUNCTION
    */
   auto context::make(const create_info& info) -> util::result<context>
   {
      return query_vulkan_version(load_core(info.logger))
         .and_then(create_instance)
         .and_then(create_debug_utils)
         .map([](context_data data) {
            context ctx{};
            ctx.m_loader = std::move(data.dynamic_loader);
            ctx.m_instance = std::move(data.instance);
            ctx.m_debug_utils = std::move(data.debug_utils);
            ctx.m_api_version = data.api_version;
            ctx.m_logger = data.logger;

            return ctx;
         });
   }

   auto context::instance() const noexcept -> vk::Instance { return m_instance.get(); }
   auto context::version() const noexcept -> std::uint32_t { return m_api_version; }

   auto context::select_device(vk::UniqueSurfaceKHR surface) const -> util::result<device>
   {
      return enumerate_physical_devices().and_then([&](auto devices) {
         return device::select({.instance = m_instance.get(),
                                .surface = std::move(surface),
                                .available_devices = devices,
                                .vulkan_version = m_api_version,
                                .logger = m_logger});
      });
   }

   auto context::enumerate_physical_devices() const
      -> util::result<crl::dynamic_array<vk::PhysicalDevice>>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_instance->enumeratePhysicalDevices();
             })
         .map([](auto&& r) {
            return crl::dynamic_array<vk::PhysicalDevice>{std::begin(r), std::end(r)};
         })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(context_error::failed_to_enumerate_physical_devices);
         });
   }

   /**
    * HELPER FUNCTIONS
    */

   auto query_instance_layers(util::logger_wrapper logger)
      -> crl::dynamic_array<vk::LayerProperties>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return vk::enumerateInstanceLayerProperties();
             })
         .join(
            [](const auto& data) {
               return crl::dynamic_array<vk::LayerProperties>{std::begin(data), std::end(data)};
            },
            [&](const auto& err) {
               if constexpr (enable_validation_layers)
               {
                  logger.warning("[vulkan] instance layer enumeration error: {0}", err.what());
               }
               return crl::dynamic_array<vk::LayerProperties>{};
            });
   }

   auto query_instance_extensions(util::logger_wrapper logger)
      -> crl::dynamic_array<vk::ExtensionProperties>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return vk::enumerateInstanceExtensionProperties();
             })
         .join(
            [](const auto& data) {
               return crl::dynamic_array<vk::ExtensionProperties>{std::begin(data), std::end(data)};
            },
            [&](const auto& err) {
               logger.warning("[vulkan] instance layer enumeration error: {1}", err.what());
               return crl::dynamic_array<vk::ExtensionProperties>{};
            });
   }

   auto has_debug_utils_extension(std::span<const vk::ExtensionProperties> properties) -> bool
   {
      return std::ranges::find_if(properties, [](const vk::ExtensionProperties& ext) {
                return strcmp(static_cast<const char*>(ext.extensionName),
                              VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
             }) != properties.end();
   }

   auto has_windowing_extensions(std::span<const vk::ExtensionProperties> properties) -> bool
   {
      const auto check_ext = [&](const char* name) -> bool {
         return std::ranges::find_if(properties, [name](const auto& ext) {
                   return strcmp(name, static_cast<const char*>(ext.extensionName)) == 0;
                }) != std::end(properties);
      };

#if defined(__linux__)
      return check_ext("VK_KHR_xcb_surface") || check_ext("VK_KHR_xlib_surface") ||
         check_ext("VK_KHR_wayland_surface");
#elif defined(_WIN32)
      return check_ext("VK_KHR_win32_surface");
#else
      return false;
#endif
   }

   auto has_khronos_validation(std::span<const vk::LayerProperties> properties) -> bool
   {
      return std::ranges::find_if(properties, [](const vk::LayerProperties& layer) {
                return strcmp(static_cast<const char*>(layer.layerName),
                              "VK_LAYER_KHRONOS_validation") == 0;
             }) != properties.end();
   }

   auto load_core(util::logger_wrapper logger) -> context_data
   {
      vk::DynamicLoader loader{};

      VULKAN_HPP_DEFAULT_DISPATCHER.init(
         loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

      logger.info("[vulkan] core functionalities loaded");

      return context_data{.dynamic_loader = std::move(loader), .logger = logger};
   }

   auto query_vulkan_version(context_data&& data) -> util::result<context_data>
   {
      return monad::try_wrap<vk::SystemError>([] {
                return vk::enumerateInstanceVersion(); // may throw
             })
         .map_error([&](vk::SystemError&& e) {
            data.logger.error("[vulkan] {}", e.what());

            return to_err_code(context_error::failed_to_query_vulkan_version);
         })
         .and_then([&](std::uint32_t v) -> util::result<context_data> {
            data.logger.info("[vulkan] vulkan version {}.{}.{} found", VK_VERSION_MAJOR(v),
                             VK_VERSION_MINOR(v), VK_VERSION_PATCH(v));

            if (VK_VERSION_MINOR(v) >= 1)
            {
               data.api_version = v;

               return std::move(data);
            }

            return monad::err(to_err_code(context_error::vulkan_version_1_1_unavailable));
         });
   }

   auto create_instance(context_data&& data) -> util::result<context_data>
   {
      const auto layers = query_instance_layers(data.logger);
      auto extensions = query_instance_extensions(data.logger);

      if (!has_windowing_extensions(extensions))
      {
         return monad::err(to_err_code(context_error::window_extensions_not_present));
      }

      crl::dynamic_array<const char*> layer_names;
      if constexpr (enable_validation_layers)
      {
         if (has_khronos_validation(layers))
         {
            layer_names.append("VK_LAYER_KHRONOS_validation");
         }
         else
         {
            data.logger.warning("[vulkan] Khronos validation layers not found");
         }
      }

      // clang-format off
      crl::dynamic_array<const char*> ext_names;
      ext_names.reserve(std::size(extensions));

      for(auto& extension : extensions)
      {
         ext_names.append(extension.extensionName);
      }
      // clang-format on

      for (const char* name : ext_names)
      {
         data.logger.info("[vulkan] instance extension: {}", name);
      }

      for (const char* name : layer_names)
      {
         data.logger.info("[vulkan] instance layer: {}", name);
      }

      const auto app_info = vk::ApplicationInfo{}.setApiVersion(data.api_version);

      return monad::try_wrap<vk::SystemError>([&] {
                return vk::createInstanceUnique(
                   vk::InstanceCreateInfo{}
                      .setPApplicationInfo(&app_info)
                      .setPEnabledLayerNames(vml::to_array_proxy<const char* const>(layer_names))
                      .setPEnabledExtensionNames(
                         vml::to_array_proxy<const char* const>(ext_names)));
             })
         .map_error([&](vk::SystemError&& e) {
            data.logger.error("[vulkan] instance error: {}", e.what());

            return to_err_code(context_error::failed_to_create_instance);
         })
         .map([&](vk::UniqueInstance instance) {
            VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

            data.logger.info("[vulkan] instance created");
            data.logger.info("[vulkan] all instance functions have been loaded");

            data.instance = std::move(instance);
            data.enabled_extensions = std::move(extensions);

            return std::move(data);
         });
   }

   auto create_debug_utils(context_data&& data) -> util::result<context_data>
   {
      if constexpr (enable_validation_layers)
      {
         return monad::try_wrap<vk::SystemError>([&] {
                   return data.instance->createDebugUtilsMessengerEXTUnique(
                      vk::DebugUtilsMessengerCreateInfoEXT{}
                         .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
                         .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                         vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                         vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
                         .setPfnUserCallback(debug_callback)
                         .setPUserData(static_cast<void*>(data.logger.get())));
                })
            .map_error([]([[maybe_unused]] const auto& err) -> util::error_t {
               return to_err_code(context_error::failed_to_create_debug_utils);
            })
            .map([&](vk::UniqueDebugUtilsMessengerEXT handle) {
               data.logger.info("[vulkan] debug utils created");

               data.debug_utils = std::move(handle);

               return std::move(data);
            });
      }
      else
      {
         return std::move(data);
      }
   }

   namespace detail
   {
      struct context_error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override
         {
            return "vulkan_context";
         }
         [[nodiscard]] auto message(int err) const -> std::string override
         {
            return to_string(static_cast<context_error>(err));
         }
      };

      static const context_error_category context_error_cat{};
   } // namespace detail

   auto to_string(context_error err) -> std::string
   {
      if (err == context_error::failed_to_initialize_glslang)
      {
         return "failed_to_initialize_glslang";
      }
      if (err == context_error::failed_to_query_vulkan_version)
      {
         return "failed_to_query_vulkan_version";
      }
      if (err == context_error::vulkan_version_1_1_unavailable)
      {
         return "vulkan_version_1_1_unavailable";
      }
      if (err == context_error::window_extensions_not_present)
      {
         return "window_extensions_not_present";
      }
      if (err == context_error::failed_to_create_instance)
      {
         return "failed_to_create_instance";
      }
      if (err == context_error::failed_to_create_debug_utils)
      {
         return "failed_to_create_debug_utils";
      }
      if (err == context_error::failed_to_enumerate_physical_devices)
      {
         return "failed_to_enumerate_physical_devices";
      }

      return "UNKNOWN";
   }
   auto to_err_code(context_error err) -> util::error_t
   {
      return {{static_cast<int>(err), detail::context_error_cat}};
   }
} // namespace vkn
