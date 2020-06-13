/**
 * @file instance.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, June 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/vk/detail/includes.hpp"
#include "epona_core/vk/detail/result.hpp"
#include "epona_core/vk/runtime.hpp"

#include <system_error>
#include <type_traits>

namespace core::vk
{
   /**
    * @class instance instance.hpp "epona_core/vk/instance.hpp"
    * @author wmbat wmbat@protonmail.com
    * @date Monday, June 5th, 2020
    * @copyright MIT License.
    * @brief A class to hold all elements related to the Vulkan Instance.
    */
   struct instance
   {
      /**
       * @brief An enum used for error handling.
       *
       * Enum class used for error handling during instance creation
       */
      enum class error
      {
         vulkan_version_unavailable,
         vulkan_version_1_2_unavailable,
         window_extensions_not_present,
         instance_extension_not_supported,
         instance_layer_not_supported,
         failed_create_instance,
         failed_create_debug_utils
      };

   public:
      instance() = default;
      instance(const instance& other) = delete;
      instance(instance&& other) noexcept;
      ~instance();

      instance& operator=(const instance& other) = delete;
      instance& operator=(instance&& other) noexcept;

   public:
      VkInstance vk_instance{VK_NULL_HANDLE};
      VkDebugUtilsMessengerEXT vk_debug_messenger{VK_NULL_HANDLE};

      uint32_t version = 0;

      dynamic_array<const char*> extensions;

   public:
      /**
       * @brief Convert an error code to a string.
       *
       * @param[in] err The error code to convert.
       *
       * @return The string equivalent of the error code.
       */
      static std::string to_string(error err);
      /**
       * @brief Convert an error code into a
       * <a href="https://en.cppreference.com/w/cpp/error/error_code">std::error_code</a>
       *
       * @param[in] err The error code to convert.
       *
       * @return the std::error_code containing the error code.
       */
      static std::error_code make_error_code(error err);
   };

   /**
    * @class instance_builder instance.hpp "epona_core/vk/instance.hpp"
    * @author wmbat wmbat@protonmail.com
    * @date Monday, June 5th, 2020
    * @copyright MIT License.
    * @brief A class used to easily construct a instance object.
    */
   class instance_builder
   {
   public:
      instance_builder(const vk::runtime& rt, logger* const p_logger = nullptr);

      /**
       * @brief Constructs an instance using the information provided.
       *
       * @return An instance object or an error code.
       */
      vk::detail::result<instance> build();

      /**
       * @brief Set the information regarding the application name.
       *
       * @param[in] app_name The name of the application.
       */
      instance_builder& set_application_name(std::string_view app_name);
      /**
       * @brief Set the information regarding the engine name.
       *
       * @param[in] app_name The name of the engine.
       */
      instance_builder& set_engine_name(std::string_view engine_name);
      /**
       * @brief Set the information regarding the application version.
       *
       * @param[in] major The version major of the application.
       * @param[in] minor The version minor of the application.
       * @param[in] patch The version patch of the application.
       */
      instance_builder& set_application_version(uint32_t major, uint32_t minor, uint32_t patch);
      /**
       * @brief Set the information regarding the engine version.
       *
       * @param[in] major The version major of the engine.
       * @param[in] minor The version minor of the engine.
       * @param[in] patch The version patch of the engine.
       */
      instance_builder& set_engine_version(uint32_t major, uint32_t minor, uint32_t patch);

      /**
       * @brief Set the information about an instance layer to enable.
       *
       * @param[in] layer_name The name of the layer to enable.
       */
      instance_builder& enable_layer(const std::string& layer_name);
      /**
       * @brief Set the information about an instance extension to enable.
       *
       * @param[in] layer_name The name of the extension to enable.
       */
      instance_builder& enable_extension(const std::string& extension_name);

   private:
      const vk::runtime& rt;

      logger* const p_logger;

      struct info
      {
         std::string app_name{};
         std::string engine_name{};
         uint32_t app_version{0};
         uint32_t engine_version{0};

         dynamic_array<const char*> layers;
         dynamic_array<const char*> extensions;

      } info;
   };

} // namespace core::vk

namespace std
{
   template <>
   struct is_error_code_enum<core::vk::instance::error> : true_type
   {
   };
} // namespace std
