#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/vk/core.hpp"
#include "epona_core/vk/result.hpp"
#include "epona_core/vk/runtime.hpp"

#include <system_error>

namespace core::vk
{
   struct instance
   {
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
      instance() noexcept = default;
      instance(const instance& other) noexcept = delete;
      instance(instance&& other) noexcept;
      ~instance();

      instance& operator=(const instance& rhs) noexcept = delete;
      instance& operator=(instance&& rhs) noexcept;

   public:
      VkInstance vk_instance{VK_NULL_HANDLE};
      VkDebugUtilsMessengerEXT vk_debug_messenger{VK_NULL_HANDLE};

      uint32_t version = 0;

   public:
      static std::string to_string(error err);
      static std::error_code make_error_code(error err);
   };

   class instance_builder
   {
   public:
      vk::details::result<instance> build(
         const vk::runtime& runtime, logger* const p_logger = nullptr);

      instance_builder& set_application_name(std::string_view app_name);
      instance_builder& set_engine_name(std::string_view engine_name);
      instance_builder& set_application_version(uint32_t major, uint32_t minor, uint32_t patch);
      instance_builder& set_engine_version(uint32_t major, uint32_t minor, uint32_t patch);

      instance_builder& enable_layer(const std::string& layer_name);
      instance_builder& enable_extension(const std::string& extension_name);

   private:
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
