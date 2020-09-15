/**
 * @file instance.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of June, 2020
 * @copyright MIT License
 */

#pragma once

#include <vkn/core.hpp>

#include <util/concepts.hpp>
#include <util/containers/dynamic_array.hpp>

#include <ranges>
#include <span>

#include <iostream>

namespace vkn
{
   /**
    * Contains all possible error values comming from the instance class.
    */
   enum class instance_error
   {
      vulkan_version_unavailable,
      vulkan_version_1_2_unavailable,
      window_extensions_not_present,
      instance_extension_not_supported,
      instance_layer_not_supported,
      failed_to_create_instance,
      failed_to_create_debug_utils
   };

   /**
    * Holds all data related to the vulkan instance
    */
   class instance final : public owning_handle<vk::Instance>
   {
   public:
      [[nodiscard]] auto version() const noexcept -> std::uint32_t;
      /**
       * Get all extensions that have been enabled in the instance
       */
      [[nodiscard]] auto extensions() const -> const util::dynamic_array<const char*>&;

   private:
      vk::UniqueInstance m_instance{nullptr};
      vk::UniqueDebugUtilsMessengerEXT m_debug_utils{nullptr};

      util::dynamic_array<const char*> m_extensions{};

      uint32_t m_version{0U};

   public:
      /**
       * A class to help in the construction of an instance object.
       */
      class builder
      {
      public:
         explicit builder(const loader& vk_loader,
                          std::shared_ptr<util::logger> p_logger = nullptr);

         /**
          * Attempt to build a command_pool object. If something goes wrong, an error
          * will be returned instead
          */
         [[nodiscard]] auto build() -> vkn::result<instance>;

         /**
          * Set the name of the application
          */
         auto set_application_name(std::string_view app_name) -> builder&;
         /**
          * Set the name of the engine
          */
         auto set_engine_name(std::string_view engine_name) -> builder&;
         /**
          * Set the version of the application
          */
         auto set_application_version(uint32_t major, uint32_t minor, uint32_t patch) -> builder&;
         /**
          * Set the version of the engine
          */
         auto set_engine_version(uint32_t major, uint32_t minor, uint32_t patch) -> builder&;
         /**
          * Set a layers to enable at instance creation
          */
         auto enable_layer(std::string_view layer_name) -> builder&;
         /**
          * Set an extension to enabled at instance creation
          */
         auto enable_extension(std::string_view extension_name) -> builder&;

      private:
         [[nodiscard]] auto build_debug_utils(vk::Instance inst) const noexcept
            -> vkn::result<vk::UniqueDebugUtilsMessengerEXT>;

         [[nodiscard]] auto
         has_validation_layer_support(std::span<const vk::LayerProperties> properties) const
            -> bool;
         [[nodiscard]] auto
         has_debug_utils_support(std::span<const vk::ExtensionProperties> properties) const -> bool;

         [[nodiscard]] auto get_all_ext(std::span<const vk::ExtensionProperties> properties,
                                        bool are_debug_utils_available) const
            -> vkn::result<util::dynamic_array<const char*>>;

         const loader& m_loader;

         std::shared_ptr<util::logger> const mp_logger;

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

   /**
    * Convert an instance_error enum to a string
    */
   auto to_string(instance_error err) -> std::string;
   /**
    * Convert an instance_error enum value and an error code from a vulkan error into
    * a vkn::error
    */
   auto make_error(instance_error err, std::error_code ec) noexcept -> vkn::error;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::instance_error> : true_type
   {
   };
} // namespace std
