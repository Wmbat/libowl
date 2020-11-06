#pragma once

#include <vulkan/device.hpp>
#include <vulkan/error.hpp>

#include <util/containers/dynamic_array.hpp>
#include <util/logger.hpp>

#include <vulkan/vulkan.hpp>

namespace vulkan
{
#if defined(NDEBUG)
   static constexpr bool enable_validation_layers = false;
#else
   static constexpr bool enable_validation_layers = true;
#endif

   enum struct context_error
   {
      failed_to_query_vulkan_version,
      vulkan_version_1_2_unavailable,
      window_extensions_not_present,
      failed_to_create_instance,
      failed_to_create_debug_utils,
      failed_to_enumerate_physical_devices,
   };

   auto to_string(context_error err) -> std::string;

   class context
   {
   public:
      struct create_info
      {
         std::shared_ptr<util::logger> p_logger{nullptr};
      };

      static auto make(const create_info& info) -> result<context>;

   public:
      [[nodiscard]] auto instance() const noexcept -> vk::Instance;

      [[nodiscard]] auto select_device(vk::UniqueSurfaceKHR surface) const -> result<device>;

   private:
      [[nodiscard]] auto enumerate_physical_devices() const
         -> result<util::dynamic_array<vk::PhysicalDevice>>;

   private:
      vk::DynamicLoader m_loader{};

      vk::UniqueInstance m_instance{nullptr};
      vk::UniqueDebugUtilsMessengerEXT m_debug_utils{nullptr};

      std::uint32_t m_api_version{};

      util::dynamic_array<vk::ExtensionProperties> m_enabled_extensions{};

      std::shared_ptr<util::logger> mp_logger{nullptr};
   };
} // namespace vulkan
