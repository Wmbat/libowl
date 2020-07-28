#include "vkn/instance.hpp"

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
            log_error(p_logger, "{0} - {1}", type, p_callback_data->pMessage);
         }
         else
         {
            log_error(p_logger, "{0}", p_callback_data->pMessage);
         }
      }
      else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
      {
         if (!type.empty())
         {
            log_warn(p_logger, "{0} - {1}", type, p_callback_data->pMessage);
         }
         else
         {
            log_warn(p_logger, "{0}", p_callback_data->pMessage);
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
      auto to_string(instance::error err) -> std::string
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
   } // namespace detail

   auto instance::error_category::name() const noexcept -> const char* { return "vk_instance"; }
   auto instance::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<instance::error>(err));
   }

   /* INSTANCE */

   instance::instance(vk::Instance instance, vk::DebugUtilsMessengerEXT debug_utils,
      util::dynamic_array<const char*> extension, uint32_t version) :
      m_instance{instance},
      m_debug_utils{debug_utils}, m_extensions(std::move(extension)), m_version(version)
   {}
   instance::instance(instance&& rhs) noexcept { *this = std::move(rhs); }
   instance::~instance()
   {
      if (m_instance)
      {
         if (m_debug_utils)
         {
            m_instance.destroyDebugUtilsMessengerEXT(m_debug_utils);
         }

         m_instance.destroy();
      }
   }

   auto instance::operator=(instance&& rhs) noexcept -> instance&
   {
      if (this != &rhs)
      {
         m_instance = rhs.m_instance;
         rhs.m_instance = nullptr;

         m_debug_utils = rhs.m_debug_utils;
         rhs.m_debug_utils = nullptr;

         m_extensions = std::move(rhs.m_extensions);
         m_version = rhs.m_version;
      }

      return *this;
   }

   auto instance::value() noexcept -> vk::Instance& { return m_instance; }
   auto instance::value() const noexcept -> const vk::Instance& { return m_instance; }
   auto instance::version() const noexcept -> uint32_t { return m_version; }
   auto instance::extensions() const -> const util::dynamic_array<const char*>&
   {
      return m_extensions;
   }

   /* INSTANCE BUILDER */
   using builder = instance::builder;

   builder::builder(const loader& vk_loader, util::logger* const plogger) :
      m_loader{vk_loader}, m_plogger{plogger}
   {}

   auto builder::build() -> vkn::result<instance>
   {
      using err_t = vkn::error;

      const auto version_res = monad::try_wrap<vk::SystemError>([] {
         return vk::enumerateInstanceVersion(); // may throw
      });

      if (!version_res)
      {
         // clang-format off
         return monad::make_left(err_t{
            .type = instance::make_error_code(instance::error::vulkan_version_unavailable),
            .result = static_cast<vk::Result>(version_res.left()->code().value())
         });
         // clang-format on
      }

      const auto system_layers_res = monad::try_wrap<vk::SystemError>([&] {
         return vk::enumerateInstanceLayerProperties();
      });

      if constexpr (detail::ENABLE_VALIDATION_LAYERS)
      {
         if (!system_layers_res)
         {
            log_warn(m_plogger, "[vkn] instance layer enumeration error: {0}",
               system_layers_res.left()->what());
         }
      }

      const auto system_exts_res = monad::try_wrap<vk::SystemError>([&] {
         return vk::enumerateInstanceExtensionProperties();
      });

      if (!system_layers_res)
      {
         log_warn(m_plogger, "[vkn] instance layer enumeration error: {1}",
            system_exts_res.left()->what());
      }

      const auto sys_layers = system_layers_res.right().value();
      const auto sys_exts = system_exts_res.right().value();

      const uint32_t api_version = version_res.right().value();
      const bool validation_layers_available = has_validation_layer_support(sys_layers);
      const bool debug_utils_available = has_debug_utils_support(sys_exts);

      log_info(m_plogger, "[vkn] using vulkan version {0}.{1}.{2}", VK_VERSION_MAJOR(api_version),
         VK_VERSION_MINOR(api_version), VK_VERSION_PATCH(api_version));

      if (VK_VERSION_MINOR(api_version) < 2)
      {
         // clang-format off
         return monad::make_left(err_t{
            .type = instance::make_error_code(instance::error::vulkan_version_1_2_unavailable),
            .result = {}
         });
         // clang-format on
      }

      // clang-format off
      const auto app_info = vk::ApplicationInfo{}
         .setPNext(nullptr)
         .setPApplicationName(m_info.app_name.c_str())
         .setApplicationVersion(m_info.app_version)
         .setPEngineName(m_info.engine_name.c_str())
         .setEngineVersion(m_info.engine_version)
         .setApiVersion(api_version);
      // clang-format on

      auto extension_names_res =
         get_all_ext(util::dynamic_array<vk::ExtensionProperties>{sys_exts.begin(), sys_exts.end()},
            debug_utils_available);
      if (!extension_names_res)
      {
         return monad::make_left(extension_names_res.left().value());
      }

      const util::dynamic_array<const char*> extensions = std::move(*extension_names_res.right());

      for (const char* name : extensions)
      {
         log_info(m_plogger, "[vkn] instance extension: {0} - ENABLED", name);
      }

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

      util::dynamic_array<const char*> layers{m_info.layers};
      if constexpr (detail::ENABLE_VALIDATION_LAYERS)
      {
         if (validation_layers_available)
         {
            layers.push_back("VK_LAYER_KHRONOS_validation");

            for (const char* name : layers)
            {
               const bool is_present = std::ranges::find_if(sys_layers, [name](const auto& ext) {
                  return strcmp(name, static_cast<const char*>(ext.layerName)) == 0;
               }) != sys_layers.cend();

               if (!is_present)
               {
                  // clang-format off
                  return monad::make_left(err_t{
                     .type = instance::make_error_code(instance::error::instance_layer_not_supported),
                     .result = {}
                  });
                  // clang-format on
               }
            }

            instance_create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
            instance_create_info.ppEnabledLayerNames = layers.data();
         }

         for (const char* name : layers)
         {
            log_info(m_plogger, "[vkn] instance layers: {0} - ENABLED", name);
         }
      }

      const auto res = monad::try_wrap<vk::SystemError>([&] {
         return vk::createInstance(instance_create_info);
      });

      if (!res)
      {
         // clang-format off
         return monad::make_left(err_t{
            .type = instance::make_error_code(instance::error::failed_to_create_instance),
            .result = static_cast<vk::Result>(res.left()->code().value())
         });
         // clang-format on
      }

      vk::DebugUtilsMessengerEXT vk_mess = nullptr;

      log_info(m_plogger, "[vkn] instance created");

      auto vk_inst = res.right().value();
      m_loader.load_instance(vk_inst);

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
            .setPUserData(static_cast<void*>(m_plogger));
         // clang-format on

         try
         {
            vk_mess = vk_inst.createDebugUtilsMessengerEXT(debug_create_info);
         }
         catch (const vk::SystemError& e)
         {
            // clang-format off
            return monad::make_left(err_t{
               .type = instance::make_error_code(instance::error::failed_to_create_debug_utils),
               .result = static_cast<vk::Result>(e.code().value())
            });
            // clang-format on
         }

         log_info(m_plogger, "[vkn] debug utils created");
      }

      return monad::make_right(instance{vk_inst, vk_mess, extensions, api_version});
   } // namespace core::gfx::vkn

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

   auto builder::has_validation_layer_support(
      const util::range_over<vk::LayerProperties> auto& properties) const -> bool
   {
      return std::ranges::find_if(properties, [](const VkLayerProperties& layer) {
         return strcmp(static_cast<const char*>(layer.layerName), "VK_LAYER_KHRONOS_validation") ==
            0;
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
      bool are_debug_utils_available) const -> vkn::result<util::dynamic_array<const char*>>
   {
      using err_t = vkn::error;

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
         // clang-format off
         return monad::make_left(err_t{
            .type = instance::make_error_code(instance::error::window_extensions_not_present),
            .result = {}
         });
         // clang-format on
      }

      for (const char* name : extensions)
      {
         const bool is_present = std::ranges::find_if(properties, [name](const auto& ext) {
            return strcmp(name, static_cast<const char*>(ext.extensionName)) == 0;
         }) != properties.cend();

         if (!is_present)
         {
            // clang-format off
            return monad::make_left(err_t{
               .type = instance::make_error_code(instance::error::instance_extension_not_supported),
               .result = {}
            });
            // clang-format on
         }
      }

      return monad::make_right(extensions);
   }
} // namespace vkn
