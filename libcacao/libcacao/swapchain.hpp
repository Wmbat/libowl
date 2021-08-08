#pragma once

#include <libcacao/device.hpp>
#include <libcacao/export.hpp>
#include <libcacao/surface.hpp>

#include <libmannele/dimension.hpp>

#include <libreglisse/result.hpp>

#include <vector>

namespace cacao
{
   class LIBCACAO_SYMEXPORT swapchain;

   struct LIBCACAO_SYMEXPORT swapchain_create_info
   {
      const cacao::device& device;
      const cacao::surface& surface;

      std::vector<vk::SurfaceFormatKHR> desired_formats;
      std::vector<vk::PresentModeKHR> desired_present_modes;

      mannele::dimension_u32 desired_dimensions;

      mannele::u32 graphics_queue_index;
      mannele::u32 present_queue_index;

      vk::ImageUsageFlags image_usage_flags{vk::ImageUsageFlagBits::eColorAttachment};
      vk::CompositeAlphaFlagBitsKHR composite_alpha_flags{vk::CompositeAlphaFlagBitsKHR::eOpaque};

      vk::Bool32 should_clip;

      swapchain* old_swapchain;

      util::log_ptr logger;
   };

   class LIBCACAO_SYMEXPORT swapchain
   {
      static constexpr std::size_t expected_image_count = 3U;

   public:
      swapchain(const swapchain_create_info& info);
      swapchain(swapchain_create_info&& info);

      [[nodiscard]] auto value() const noexcept -> vk::SwapchainKHR;
      [[nodiscard]] auto format() const noexcept -> vk::Format;
      [[nodiscard]] auto extent() const noexcept -> const vk::Extent2D&;
      [[nodiscard]] auto images() const noexcept -> std::span<const vk::Image>;

   private:
      vk::UniqueSwapchainKHR m_swapchain;
      std::vector<vk::Image> m_images;

      vk::Format m_format{};
      vk::Extent2D m_extent{};

      util::log_ptr m_logger;
   };

   auto LIBCACAO_SYMEXPORT query_surface_support(const device& device, const surface& surface)
      -> reglisse::result<surface_support, surface_support_error>;
} // namespace cacao
