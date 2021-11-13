/**
 * @file libcacao/swapchain.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_SWAPCHAIN_HPP_
#define LIBCACAO_SWAPCHAIN_HPP_

#include <libcacao/device.hpp>
#include <libcacao/export.hpp>

// Third Party Libraries

#include <libmannele/dimension.hpp>

#include <libreglisse/result.hpp>

// C++ Standard Library

#include <vector>

namespace cacao
{
   class LIBCACAO_SYMEXPORT swapchain;

   struct LIBCACAO_SYMEXPORT swapchain_create_info
   {
      const cacao::device& device;

      vk::SurfaceKHR surface;

      std::vector<vk::SurfaceFormatKHR> desired_formats;
      std::vector<vk::PresentModeKHR> desired_present_modes;

      mannele::dimension_u32 desired_dimensions;

      mannele::u32 graphics_queue_index;
      mannele::u32 present_queue_index;

      vk::ImageUsageFlags image_usage_flags{vk::ImageUsageFlagBits::eColorAttachment};
      vk::CompositeAlphaFlagBitsKHR composite_alpha_flags{vk::CompositeAlphaFlagBitsKHR::eOpaque};

      vk::Bool32 should_clip;

      swapchain* old_swapchain;

      mannele::log_ptr logger;
   };

   class LIBCACAO_SYMEXPORT swapchain
   {
      static constexpr std::size_t expected_image_count = 3U;

   public:
      swapchain() = default;
      explicit swapchain(const swapchain_create_info& info);

      [[nodiscard]] auto value() const noexcept -> vk::SwapchainKHR;
      [[nodiscard]] auto format() const noexcept -> vk::Format;
      [[nodiscard]] auto extent() const noexcept -> const vk::Extent2D&;
      [[nodiscard]] auto images() const noexcept -> std::span<const vk::Image>;

      // TODO(wmbat): Do image views belong in the swapchain class?
      [[nodiscard]] auto image_views() const noexcept -> std::span<const vk::UniqueImageView>;

   private:
      vk::UniqueSwapchainKHR m_swapchain;
      std::vector<vk::Image> m_images;
      std::vector<vk::UniqueImageView> m_image_views;

      vk::Format m_format{};
      vk::Extent2D m_extent{};

      mannele::log_ptr m_logger;
   };

   struct surface_support
   {
      vk::SurfaceCapabilitiesKHR capabilities;
      std::vector<vk::SurfaceFormatKHR> formats;
      std::vector<vk::PresentModeKHR> present_modes;
   };
 
   enum class surface_support_error
   {
      failed_to_get_surface_capabilities,
      failed_to_enumerate_formats,
      failed_to_enumerate_present_modes
   };

   auto LIBCACAO_SYMEXPORT to_error_condition(surface_support_error code) -> std::error_condition;

   auto LIBCACAO_SYMEXPORT query_surface_support(const device& device, vk::SurfaceKHR surface)
      -> reglisse::result<surface_support, surface_support_error>;
} // namespace cacao

#endif // LIBCACAO_SWAPCHAIN_HPP_
