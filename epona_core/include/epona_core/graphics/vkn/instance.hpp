/**
 * @file instance.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of June, 2020
 * @copyright MIT License
 */

#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/graphics/vkn/core.hpp"

#include <ranges>

namespace core::gfx::vkn
{
   /**
    * @class instance <epona_core/graphics/vkn/instance.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Saturday, 20th of June, 2020
    * @copyright MIT License
    *
    * @brief A wrapper struct around the elements associated with a Vulkan instance
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
         failed_to_create_instance,
         failed_to_create_debug_utils
      };

      vk::UniqueInstance inst;
      vk::UniqueDebugUtilsMessengerEXT debug_utils;

      tiny_dynamic_array<const char*, 16> extensions;

      uint32_t version = 0;
   };

   /**
    * @class instance <epona_core/graphics/vkn/instance.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Saturday, 20th of June, 2020
    * @copyright MIT License
    *
    * @brief A class used for a parametrized creation of an instance.
    */
   class instance_builder
   {
   public:
      instance_builder(const loader& vk_loader, logger* const p_logger = nullptr);

      /**
       * @brief Constructs an instance using the information provided.
       *
       * @return An instance object or an error code.
       */
      result<instance> build();

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
      instance_builder& enable_layer(std::string_view layer_name);
      /**
       * @brief Set the information about an instance extension to enable.
       *
       * @param[in] layer_name The name of the extension to enable.
       */
      instance_builder& enable_extension(std::string_view extension_name);

   private:
      bool has_validation_layer_support(const dynamic_array<vk::LayerProperties>& properties) const;
      bool has_debug_utils_support(const dynamic_array<vk::ExtensionProperties>& properties) const;

      result<tiny_dynamic_array<const char*, 16>> get_all_ext(
         const dynamic_array<vk::ExtensionProperties>& properties,
         bool are_debug_utils_available) const;

   private:
      const loader& vk_loader;

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
} // namespace core::gfx::vkn

namespace std
{
   template <>
   struct is_error_code_enum<core::gfx::vkn::instance::error> : true_type
   {
   };
} // namespace std
