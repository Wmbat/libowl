/**
 * @file instance.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of June, 2020
 * @copyright MIT License
 */

#pragma once

#include <core/graphics/vkn/core.hpp>

#include <util/concepts.hpp>
#include <util/containers/dynamic_array.hpp>

#include <ranges>

#include <iostream>

namespace core::gfx::vkn
{
   /**
    * @class instance <epona_core/graphics/vkn/instance.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Saturday, 20th of June, 2020
    * @copyright MIT License
    */
   class instance
   {
   public:
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

      instance() = default;
      instance(vk::Instance instance, vk::DebugUtilsMessengerEXT debug_utils,
         util::dynamic_array<const char*> extension, uint32_t version);
      instance(const instance&) = delete;
      instance(instance&&) noexcept;
      ~instance();

      auto operator=(const instance&) -> instance& = delete;
      auto operator=(instance&&) noexcept -> instance&;

      auto value() noexcept -> vk::Instance&;
      [[nodiscard]] auto value() const noexcept -> const vk::Instance&;

      [[nodiscard]] auto extensions() const -> const util::dynamic_array<const char*>&;

   private:
      vk::Instance m_instance;
      vk::DebugUtilsMessengerEXT m_debug_utils;

      util::dynamic_array<const char*> m_extensions;

      uint32_t m_version = 0;

   public:
      class builder
      {
      public:
         builder(const loader& vk_loader, util::logger* const p_logger = nullptr);

         [[nodiscard]] auto build() -> vkn::result<instance>;

         auto set_application_name(std::string_view app_name) -> builder&;
         auto set_engine_name(std::string_view engine_name) -> builder&;
         auto set_application_version(uint32_t major, uint32_t minor, uint32_t patch) -> builder&;
         auto set_engine_version(uint32_t major, uint32_t minor, uint32_t patch) -> builder&;
         auto enable_layer(std::string_view layer_name) -> builder&;
         auto enable_extension(std::string_view extension_name) -> builder&;

      private:
         auto has_validation_layer_support(
            const util::range_over<vk::LayerProperties> auto& properties) const -> bool;
         auto has_debug_utils_support(
            const util::range_over<vk::ExtensionProperties> auto& properties) const -> bool;

         [[nodiscard]] auto get_all_ext(
            const util::dynamic_array<vk::ExtensionProperties>& properties,
            bool are_debug_utils_available) const -> vkn::result<util::dynamic_array<const char*>>;

      private:
         const loader& m_loader;

         util::logger* const m_plogger;

         struct info
         {
            std::string app_name{};
            std::string engine_name{};
            uint32_t app_version{0};
            uint32_t engine_version{0};

            util::dynamic_array<const char*> layers;
            util::dynamic_array<const char*> extensions;
         } m_info;
      };
   };
} // namespace core::gfx::vkn

namespace std
{
   template <>
   struct is_error_code_enum<core::gfx::vkn::instance::error> : true_type
   {
   };
} // namespace std
