/**
 * @file libowl/gfx/render_target.cpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief
 * @copyright Copyright (C) 2022 wmbat
 */

#include <libowl/gfx/render_target.hpp>

#include <ranges>

namespace stdr = std::ranges;

namespace
{
   static constexpr std::array desired_formats = {
      vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear),
      vk::SurfaceFormatKHR(vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear)};

   static constexpr std::array desired_present_modes = {vk::PresentModeKHR::eMailbox,
                                                        vk::PresentModeKHR::eFifo};

   auto find_best_surface_format(std::span<const vk::SurfaceFormatKHR> available_formats)
      -> vk::SurfaceFormatKHR
   {
      const auto it = stdr::find_first_of(available_formats, desired_formats);
      return it != stdr::end(available_formats) ? *it : available_formats[0];
   }

   auto find_best_present_mode(std::span<const vk::PresentModeKHR> available_modes)
      -> vk::PresentModeKHR
   {
      const auto it = stdr::find_first_of(available_modes, desired_present_modes);
      return it != stdr::end(available_modes) ? *it : vk::PresentModeKHR::eFifo;
   }
} // namespace

namespace owl::inline v0
{
   render_target::render_target(vk::UniqueSurfaceKHR&& surface) : m_surface(std::move(surface)) {}

   [[nodiscard]] auto render_target::surface() const noexcept -> vk::SurfaceKHR
   {
      // NOLINTNEXTLINE
      assert(static_cast<VkSurfaceKHR>(m_surface.get()) != VK_NULL_HANDLE);

      return m_surface.get();
   }

   void render_target::set_device(gfx::device&& device)
   {
      if (m_device != device)
      {
         if (m_device)
         {
            m_status = target_status::device_lost;
         }

         m_device = std::move(device);
      }
   }

   auto render_target::create_swapchain() -> vk::UniqueSwapchainKHR
   {
      // NOLINTNEXTLINE
      assert(m_device.has_value());

      const auto surface_format = find_best_surface_format({});
      const auto present_mode = find_best_present_mode({});

      return static_cast<vk::Device>(m_device->logical())
         .createSwapchainKHRUnique(vk::SwapchainCreateInfoKHR()
                                      .setSurface(m_surface.get())
                                      .setPresentMode(present_mode)
                                      .setImageFormat(surface_format.format)
                                      .setImageColorSpace(surface_format.colorSpace));
   }
} // namespace owl::inline v0
