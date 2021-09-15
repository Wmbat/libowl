/**
 * @file libcacao/context.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_CONTEXT_HPP_
#define LIBCACAO_CONTEXT_HPP_

#include <libcacao/export.hpp>
#include <libcacao/vulkan.hpp>

// Third Party Libraries

#include <libmannele/logging/log_ptr.hpp>

// Standard Library

#include <span>
#include <vector>

namespace cacao
{
#if defined(NDEBUG)
   static constexpr bool enable_validation_layers = false;
#else
   static constexpr bool enable_validation_layers = true;
#endif

   struct LIBCACAO_SYMEXPORT context_create_info
   {
      std::uint32_t min_vulkan_version = VK_MAKE_VERSION(1, 1, 0);
      bool use_window = true;

      mannele::log_ptr logger{nullptr};
   };

   class LIBCACAO_SYMEXPORT context
   {
   public:
      context() = default;
      explicit context(context_create_info&& info);

      [[nodiscard]] auto instance() const noexcept -> vk::Instance;
      [[nodiscard]] auto vulkan_version() const noexcept -> std::uint32_t;

      [[nodiscard]] auto enumerate_physical_devices() const -> std::vector<vk::PhysicalDevice>;

   private:
      [[nodiscard]] auto load_vulkan_core(mannele::log_ptr logger) const -> vk::DynamicLoader;
      [[nodiscard]] auto get_vulkan_api_version(std::uint32_t minimum_version) const
         -> std::uint32_t;
      [[nodiscard]] auto create_instance(const context_create_info& info) const
         -> vk::UniqueInstance;
      [[nodiscard]] auto create_debug_utils(mannele::log_ptr logger) const
         -> vk::UniqueDebugUtilsMessengerEXT;

   private:
      vk::DynamicLoader m_loader{};

      std::uint32_t m_api_version{};

      vk::UniqueInstance m_instance{nullptr};
      vk::UniqueDebugUtilsMessengerEXT m_debug_utils{nullptr};

      mannele::log_ptr m_logger;
   };

   auto LIBCACAO_SYMEXPORT check_layer_support(std::span<const vk::LayerProperties> layers,
                                               std::string_view name) -> bool;
   auto LIBCACAO_SYMEXPORT check_extension_support(
      std::span<const vk::ExtensionProperties> extensions, std::string_view name) -> bool;
} // namespace cacao

#endif // LIBCACAO_CONTEXT_HPP_
