/**
 * @file libash/instance.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBASH_INSTANCE_HPP_
#define LIBASH_INSTANCE_HPP_

#include <libash/core.hpp>
#include <libash/detail/vulkan.hpp>

#include <libmannele/core/semantic_version.hpp>
#include <libmannele/logging/log_ptr.hpp>

#include <span>
#include <string_view>

namespace ash::inline v0
{
   namespace detail
   {
      auto is_layer_available(std::string_view name,
                              std::span<const vk::LayerProperties> available_layers) -> bool;
      auto is_extension_available(std::string_view name,
                                  std::span<const vk::ExtensionProperties> extensions) -> bool;
   } // namespace detail

   /**
    * @brief Possible error that can be encountered when creating an ash::instance
    */
   enum struct instance_error
   {
      window_support_not_found,
      layer_support_not_found,
      extension_support_not_found
   };

   /**
    * @brief Convert an ash::instance_error value into a std::error_condition.
    *
    * @param[in] The error code to convert
    */
   auto to_error_condition(instance_error err) -> std::error_condition;

   /**
    * @brief Simple struct holding the user's application name and version.
    */
   struct application_info
   {
      std::string_view name;
      mannele::semantic_version version;
   };

   /**
    * @brief Simple struct holding the user's engine name and version.
    */
   struct engine_info
   {
      std::string_view name;
      mannele::semantic_version version;
   };

   /**
    * @brief Data used for the creation of an ash::instance object.
    */
   struct instance_create_info
   {
      application_info app_info;
      engine_info eng_info;

      bool is_headless = false;

      std::vector<const char*> enabled_extension_names;
      std::vector<const char*> enabled_layer_names;

      mannele::log_ptr logger;
   };

   /**
    * @brief Helper class to instantiate a vulkan instance.
    */
   class instance
   {
   public:
      /**
       * @brief Construct an ash::instance using the data provided in ash::instance_create_info.
       *
       * The constructor may throw an ash::runtime_error containing a std::error_condition
       */
      instance(const instance_create_info& info);

      /**
       * @brief Implicitely converts the a vulkan vk::Instance.
       */
      operator vk::Instance() const;

      /**
       * @brief Access the underlying Vulkan instance version.
       */
      [[nodiscard]] auto version() const noexcept -> mannele::semantic_version;

   private:
      vk::DynamicLoader m_loader{};

      mannele::semantic_version m_api_version{};

      vk::UniqueInstance m_instance{nullptr};
      vk::UniqueDebugUtilsMessengerEXT m_debug_utils{nullptr};

      mannele::log_ptr m_logger;
   };
} // namespace ash::inline v0

#endif // LIBASH_INSTANCE_HPP_
