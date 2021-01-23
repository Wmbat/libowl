#pragma once

#include <utility/logger.hpp>

#include <libcaramel/containers/dynamic_array.hpp>

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#include <vulkan/vulkan.hpp>

#include <span>

namespace cacao
{
   namespace detail
   {
#if defined(NDEBUG)
      static constexpr bool enable_validation_layers = false;
#else
      static constexpr bool enable_validation_layers = true;
#endif
   } // namespace detail

   struct context_create_info
   {
      std::uint32_t min_vulkan_version = VK_MAKE_VERSION(1, 1, 0);
      bool use_window = true;

      util::logger_wrapper logger{nullptr};
   };

   class context
   {
   public:
      context() = default;
      context(context_create_info&& info);

      [[nodiscard]] auto instance() const noexcept -> vk::Instance;
      [[nodiscard]] auto vulkan_version() const noexcept -> std::uint32_t;

      [[nodiscard]] auto enumerate_physical_devices() const
         -> crl::small_dynamic_array<vk::PhysicalDevice, 2>;

   private:
      auto load_vulkan_core(util::logger_wrapper logger) const -> vk::DynamicLoader;
      auto get_vulkan_api_version(std::uint32_t minimum_version) const -> std::uint32_t;
      auto create_instance(const context_create_info& info) const -> vk::UniqueInstance;
      auto create_debug_utils(util::logger_wrapper logger) const
         -> vk::UniqueDebugUtilsMessengerEXT;

   private:
      vk::DynamicLoader m_loader{};

      std::uint32_t m_api_version{};

      vk::UniqueInstance m_instance{nullptr};
      vk::UniqueDebugUtilsMessengerEXT m_debug_utils{nullptr};

      util::logger_wrapper m_logger;
   };

   auto check_layer_support(std::span<const vk::LayerProperties> layers, std::string_view name)
      -> bool;
   auto check_extension_support(std::span<const vk::ExtensionProperties> extensions,
                                std::string_view name) -> bool;
} // namespace cacao
