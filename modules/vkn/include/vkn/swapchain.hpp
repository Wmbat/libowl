/**
 * @file swapchain.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of June, 2020
 * @copyright MIT License
 */

#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

namespace vkn
{
   class swapchain
   {
      static constexpr size_t EXPECTED_IMAGE_COUNT = 3u;

   public:
      enum class error
      {
         surface_handle_not_provided,
         failed_to_query_surface_support_details,
         failed_to_create_swapchain,
         failed_to_get_swapchain_images,
         failed_to_create_swapchain_image_views,
      };

      struct create_info
      {
         vk::Device device{nullptr};
         vk::SwapchainKHR swapchain{nullptr};
         vk::Format format{};
         vk::Extent2D extent{};

         util::tiny_dynamic_array<vk::Image, EXPECTED_IMAGE_COUNT> images;
      };

   public:
      swapchain() = default;
      swapchain(const create_info& info) noexcept;
      swapchain(const swapchain&) = delete;
      swapchain(swapchain&& rhs) noexcept;
      ~swapchain();

      auto operator=(const swapchain&) -> swapchain& = delete;
      auto operator=(swapchain&&) noexcept -> swapchain&;

      [[nodiscard]] auto get_image_views() const
         -> result<util::tiny_dynamic_array<vk::ImageView, EXPECTED_IMAGE_COUNT>>;

      [[nodiscard]] auto value() const -> const vk::SwapchainKHR&;

   private:
      vk::Device m_device{nullptr};
      vk::SwapchainKHR m_swapchain{nullptr};
      vk::Format m_format{};
      vk::Extent2D m_extent{};

      util::tiny_dynamic_array<vk::Image, EXPECTED_IMAGE_COUNT> m_images;

   public:
      class builder
      {
      public:
         builder(const device& device, util::logger* const plogger);

         [[nodiscard]] auto build() -> vkn::result<swapchain>;

         auto set_old_swapchain(const swapchain& swap) noexcept -> builder&;

         auto set_desired_extent(uint32_t width, uint32_t height) noexcept -> builder&;

         auto set_desired_format(const vk::SurfaceFormatKHR& format) -> builder&;
         auto add_fallback_format(const vk::SurfaceFormatKHR& format) -> builder&;
         auto use_default_format_selection() -> builder&;

         auto set_desired_present_mode(vk::PresentModeKHR present_mode) -> builder&;
         auto add_fallback_present_mode(vk::PresentModeKHR present_mode) -> builder&;
         auto use_default_present_mode_selection() -> builder&;

         auto set_image_usage_flags(const vk::ImageUsageFlags& usage_flags) noexcept -> builder&;
         auto add_image_usage_flags(const vk::ImageUsageFlags& usage_flags) noexcept -> builder&;
         auto use_default_image_usage_flags() noexcept -> builder&;

         auto set_composite_alpha_flags(vk::CompositeAlphaFlagBitsKHR alpha_flags) noexcept
            -> builder&;

         auto set_clipped(bool clipped = true) noexcept -> builder&;

      private:
         [[nodiscard]] auto add_desired_formats() const
            -> util::tiny_dynamic_array<vk::SurfaceFormatKHR, 2>;
         [[nodiscard]] auto add_desired_present_modes() const
            -> util::tiny_dynamic_array<vk::PresentModeKHR, 2>;

      private:
         static constexpr uint32_t DEFAULT_SIZE = 256;

         util::logger* const m_plogger;

         struct swap_info
         {
            vk::PhysicalDevice physical_device{nullptr};
            vk::Device device{nullptr};
            vk::SurfaceKHR surface{nullptr};
            vk::SwapchainKHR old_swapchain{nullptr};

            util::tiny_dynamic_array<vk::SurfaceFormatKHR, 2> desired_formats{};
            util::tiny_dynamic_array<vk::PresentModeKHR, 2> desired_present_modes{};

            uint32_t desired_width{DEFAULT_SIZE};
            uint32_t desired_height{DEFAULT_SIZE};

            uint32_t graphics_queue_index{0};
            uint32_t present_queue_index{0};

            vk::ImageUsageFlags image_usage_flags{vk::ImageUsageFlagBits::eColorAttachment};
            vk::CompositeAlphaFlagBitsKHR composite_alpha_flags{
               vk::CompositeAlphaFlagBitsKHR::eOpaque};

            vk::Bool32 clipped = VK_TRUE;
         } m_info;
      };
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::swapchain::error> : true_type
   {
   };
} // namespace std
