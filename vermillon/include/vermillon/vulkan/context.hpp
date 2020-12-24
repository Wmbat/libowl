#pragma once

#include <vermillon/vulkan/device.hpp>

#include <vermillon/util/logger.hpp>

#include <vulkan/vulkan.hpp>

namespace vkn
{
#if defined(NDEBUG)
   static constexpr bool enable_validation_layers = false;
#else
   static constexpr bool enable_validation_layers = true;
#endif

   enum struct context_error
   {
      failed_to_initialize_glslang,
      failed_to_query_vulkan_version,
      vulkan_version_1_1_unavailable,
      window_extensions_not_present,
      failed_to_create_instance,
      failed_to_create_debug_utils,
      failed_to_enumerate_physical_devices,
   };

   auto to_string(context_error err) -> std::string;
   auto to_err_code(context_error err) -> util::error_t;

   class context
   {
   public:
      struct create_info
      {
         cacao::logger_wrapper logger;
      };

      static auto make(const create_info& info) -> util::result<context>;

   public:
      [[nodiscard]] auto instance() const noexcept -> vk::Instance;
      [[nodiscard]] auto version() const noexcept -> std::uint32_t;

      [[nodiscard]] auto select_device(vk::UniqueSurfaceKHR surface) const -> util::result<device>;

   private:
      [[nodiscard]] auto enumerate_physical_devices() const
         -> util::result<crl::dynamic_array<vk::PhysicalDevice>>;

   private:
      vk::DynamicLoader m_loader{};

      vk::UniqueInstance m_instance{nullptr};
      vk::UniqueDebugUtilsMessengerEXT m_debug_utils{nullptr};

      std::uint32_t m_api_version{};

      crl::dynamic_array<vk::ExtensionProperties> m_enabled_extensions{};

      cacao::logger_wrapper m_logger;

      static inline bool is_glslang_init = false;
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::context_error> : true_type
   {
   };
} // namespace std
