/**
 * @file libowl/gfx/render_target.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief
 * @copyright Copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_GFX_RENDER_TARGET_HPP_
#define LIBOWL_GFX_RENDER_TARGET_HPP_

#include <libowl/gfx/device.hpp>

#include <optional>

namespace owl::inline v0
{
   enum struct target_status
   {
      no_device,
      no_surface,
      no_swapchain,
      device_lost,
      surface_lost,
      swapchain_lost
   };

   class render_target
   {
   public:
      render_target() = default;
      render_target(vk::UniqueSurfaceKHR&& surface);

      [[nodiscard]] auto surface() const noexcept -> vk::SurfaceKHR;

      void set_device(gfx::device&& device);

   private:
      auto create_swapchain() -> vk::UniqueSwapchainKHR;

   private:
      target_status m_status = target_status::no_device;

      std::optional<gfx::device> m_device;

      vk::UniqueSurfaceKHR m_surface;
      vk::UniqueSwapchainKHR m_swapchain;
   };
} // namespace owl::inline v0

#endif // LIBOWL_GFX_RENDER_TARGET_HPP_
