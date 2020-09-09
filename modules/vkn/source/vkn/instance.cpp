#include <vkn/instance.hpp>
#include <vulkan/vulkan.hpp>

#include <monads/try.hpp>

#include <functional>
#include <utility>

namespace vkn
{
   static VKAPI_ATTR auto VKAPI_CALL debug_callback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data) -> VkBool32
   {
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

   /**
    * A struct used for error handling and displaying error messages
    */
   struct instance_error_category : std::error_category
   {
      /**
       * The name of the vkn object the error appeared from.
       */
      [[nodiscard]] auto name() const noexcept -> const char* override { return "vkn_instance"; }
      /**
       * Get the message associated with a specific error code.
       */
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<instance_error>(err));
      }
   };

   inline static const instance_error_category instance_category{};

   auto to_string(instance_error err) -> std::string
   {
      switch (err)
      {
         case instance_error::vulkan_version_unavailable:
            return "vulkan_version_unavailable";
         case instance_error::vulkan_version_1_2_unavailable:
            return "vulkan_version_1_2_unavailable";
         case instance_error::window_extensions_not_present:
            return "window_extensions_not_present";
         case instance_error::instance_extension_not_supported:
            return "instance_extension_not_supported";
         case instance_error::instance_layer_not_supported:
            return "instance_layer_not_supported";
         case instance_error::failed_to_create_instance:
            return "failed_create_instance";
         case instance_error::failed_to_create_debug_utils:
            return "failed_create_debug_utils";
         default:
            return "UNKNOWN";
      }
   };

   auto make_error(instance_error err, std::error_code ec) noexcept -> vkn::error
   {
      return {{static_cast<int>(err), instance_category}, static_cast<vk::Result>(ec.value())};
   }

   /* INSTANCE */

   auto instance::version() const noexcept -> uint32_t { return m_version; }
   auto instance::extensions() const -> const util::dynamic_array<const char*>&
   {
      return m_extensions;
   }

   /* INSTANCE BUILDER */
   using builder = instance::builder;

   builder::builder(const loader& vk_loader, std::shared_ptr<util::logger> p_logger) :
      m_loader{vk_loader}, mp_logger{std::move(p_logger)}
   {}

   auto builder::build() -> monad::result<instance, vkn::error>
   {
      const auto version_res = monad::try_wrap<vk::SystemError>([] {
         return vk::enumerateInstanceVersion(); // may throw
      });

      if (!version_res)
      {
         return monad::make_error(
            make_error(instance_error::vulkan_version_unavailable, version_res.error()->code()));
      }

      const uint32_t api_version = version_res.value().value();

      if (VK_VERSION_MINOR(api_version) < 2)
      {
         return monad::make_error(make_error(instance_error::vulkan_version_1_2_unavailable, {}));
      }

      util::log_info(mp_logger, "[vkn] using vulkan version {}.{}.{}",
                     VK_VERSION_MAJOR(api_version), VK_VERSION_MINOR(api_version),
                     VK_VERSION_PATCH(api_version));

      // clang-format off
      const auto sys_layers = monad::try_wrap<vk::SystemError>([&] {
         return vk::enumerateInstanceLayerProperties();
      }).join(
         [](const auto& data) {
            return util::dynamic_array<vk::LayerProperties>{std::begin(data), std::end(data)};
         },
         [&](const auto& err) {
            if constexpr (detail::ENABLE_VALIDATION_LAYERS)
            {
               log_warn(mp_logger, "[vkn] instance layer enumeration error: {0}", err.what());
            }
            return util::dynamic_array<vk::LayerProperties>{};
         }
      );
  
      const auto sys_exts = monad::try_wrap<vk::SystemError>([&] {
         return vk::enumerateInstanceExtensionProperties();
      }).join(
         [](const auto& data){
            return util::dynamic_array<vk::ExtensionProperties>{std::begin(data), std::end(data)};
         },
         [&](const auto& err) {
            log_warn(mp_logger, "[vkn] instance layer enumeration error: {1}", err.what());
            return util::dynamic_array<vk::ExtensionProperties>{};
         }
      );
      // clang-format on

      // clang-format off
      const auto app_info = vk::ApplicationInfo{}
         .setPNext(nullptr)
         .setPApplicationName(m_info.app_name.c_str())
         .setApplicationVersion(m_info.app_version)
         .setPEngineName(m_info.engine_name.c_str())
         .setEngineVersion(m_info.engine_version)
         .setApiVersion(api_version);
      // clang-format on

      auto extension_names_res = get_all_ext(sys_exts, has_debug_utils_support(sys_exts));
      if (!extension_names_res)
      {
         return monad::make_error(extension_names_res.error().value());
      }

      const util::dynamic_array<const char*> extensions = std::move(*extension_names_res.value());
      for (const char* name : extensions)
      {
         log_info(mp_logger, "[vkn] instance extension: {0} - ENABLED", name);
      }

      // clang-format off
      auto instance_create_info = vk::InstanceCreateInfo{}
         .setPNext(nullptr)
         .setFlags({})
         .setPApplicationInfo(&app_info)
         .setEnabledLayerCount(0)
         .setPpEnabledLayerNames(nullptr)
         .setEnabledExtensionCount(static_cast<uint32_t>(std::size(extensions)))
         .setPpEnabledExtensionNames(extensions.data());
      // clang-format on

      util::dynamic_array<const char*> layers{m_info.layers};
      if constexpr (detail::ENABLE_VALIDATION_LAYERS)
      {
         for (const auto& name : sys_layers)
         {
            log_info(mp_logger, "[vkn] instance layers: {0} - ENABLED", name.layerName);
         }

         if (has_validation_layer_support(sys_layers))
         {
            layers.push_back("VK_LAYER_KHRONOS_validation");

            for (const char* name : layers)
            {
               const bool is_present =
                  std::ranges::find_if(sys_layers, [name](const auto& ext) {
                     return strcmp(name, static_cast<const char*>(ext.layerName)) == 0;
                  }) != sys_layers.cend();

               if (!is_present)
               {
                  return monad::make_error(
                     make_error(instance_error::instance_layer_not_supported, {}));
               }
            }

            instance_create_info.enabledLayerCount = static_cast<uint32_t>(std::size(layers));
            instance_create_info.ppEnabledLayerNames = layers.data();
         }

         for (const char* name : layers)
         {
            log_info(mp_logger, "[vkn] instance layers: {0} - ENABLED", name);
         }
      }

      return monad::try_wrap<vk::SystemError>([&] {
                return vk::createInstanceUnique(instance_create_info);
             })
         .map_error([](auto err) { // Error creating instance
            return make_error(instance_error::failed_to_create_instance, err.code());
         })
         .and_then([&](vk::UniqueInstance&& inst) { // Instance created
            log_info(mp_logger, "[vkn] instance created");
            m_loader.load_instance(inst.get());

            return build_debug_utils(inst.get()).map([&](auto debug) {
               instance value{};
               value.m_value = std::move(inst);
               value.m_debug_utils = std::move(debug);
               value.m_extensions = extensions;
               value.m_version = api_version;

               return value;
            });
         });
   } // namespace vkn

   auto builder::set_application_name(std::string_view app_name) -> builder&
   {
      m_info.app_name = app_name;
      return *this;
   }
   auto builder::set_engine_name(std::string_view engine_name) -> builder&
   {
      m_info.engine_name = engine_name;
      return *this;
   }
   auto builder::set_application_version(uint32_t major, uint32_t minor, uint32_t patch) -> builder&
   {
      m_info.app_version = VK_MAKE_VERSION(major, minor, patch);
      return *this;
   }
   auto builder::set_engine_version(uint32_t major, uint32_t minor, uint32_t patch) -> builder&
   {
      m_info.engine_version = VK_MAKE_VERSION(major, minor, patch);
      return *this;
   }
   auto instance::builder::enable_layer(std::string_view layer_name) -> builder&
   {
      if (!layer_name.empty())
      {
         m_info.layers.push_back(layer_name.data());
      }
      return *this;
   }
   auto builder::enable_extension(std::string_view extension_name) -> builder&
   {
      if (!extension_name.empty())
      {
         m_info.extensions.push_back(extension_name.data());
      }
      return *this;
   }

   auto builder::build_debug_utils(vk::Instance inst) const noexcept
      -> monad::result<vk::UniqueDebugUtilsMessengerEXT, vkn::error>
   {
      if constexpr (detail::ENABLE_VALIDATION_LAYERS)
      {
         return monad::try_wrap<vk::SystemError>([&] {
                   return inst.createDebugUtilsMessengerEXTUnique(
                      {.pNext = nullptr,
                       .flags = {},
                       .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                       .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                          vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                       .pfnUserCallback = debug_callback,
                       .pUserData = static_cast<void*>(mp_logger.get())});
                })
            .map_error([](const auto& err) -> vkn::error {
               return make_error(instance_error::failed_to_create_debug_utils, err.code());
            })
            .map([&](vk::UniqueDebugUtilsMessengerEXT handle) {
               log_info(mp_logger, "[vkn] debug utils created");
               return handle;
            });
      }
      else
      {
         return vk::UniqueDebugUtilsMessengerEXT{nullptr};
      }
   }

   auto builder::has_validation_layer_support(
      const util::range_over<vk::LayerProperties> auto& properties) const -> bool
   {
      return std::ranges::find_if(properties, [](const VkLayerProperties& layer) {
                return strcmp(static_cast<const char*>(layer.layerName),
                              "VK_LAYER_KHRONOS_validation") == 0;
             }) != properties.end();
   }

   auto instance::builder::has_debug_utils_support(
      const util::range_over<vk::ExtensionProperties> auto& properties) const -> bool
   {
      return std::ranges::find_if(properties, [](const VkExtensionProperties& ext) {
                return strcmp(static_cast<const char*>(ext.extensionName),
                              VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
             }) != properties.end();
   }

   auto builder::get_all_ext(const util::dynamic_array<vk::ExtensionProperties>& properties,
                             bool are_debug_utils_available) const
      -> monad::result<util::dynamic_array<const char*>, vkn::error>
   {
      util::dynamic_array<const char*> extensions{m_info.extensions};

      if constexpr (detail::ENABLE_VALIDATION_LAYERS)
      {
         if (are_debug_utils_available)
         {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
         }
      }

      const auto check_ext_and_add = [&](const char* name) -> bool {
         const auto it = std::ranges::find_if(properties, [name](const VkExtensionProperties& ext) {
            return strcmp(name, static_cast<const char*>(ext.extensionName)) == 0;
         });

         if (it != properties.cend())
         {
            extensions.push_back(name);

            return true;
         }

         return false;
      };

      const bool has_khr_surface_ext = check_ext_and_add("VK_KHR_surface");

      // maybe look into GLFW extensions stuff

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
         return monad::make_error(make_error(instance_error::window_extensions_not_present, {}));
      }

      for (const char* name : extensions)
      {
         const bool is_present =
            std::ranges::find_if(properties, [name](const auto& ext) {
               return strcmp(name, static_cast<const char*>(ext.extensionName)) == 0;
            }) != properties.cend();

         if (!is_present)
         {
            return monad::make_error(
               make_error(instance_error::instance_extension_not_supported, {}));
         }
      }

      return extensions;
   }
} // namespace vkn
