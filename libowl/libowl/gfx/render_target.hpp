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
#include <libowl/gui/monitor.hpp>

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

   /**
    * @brief Values to designate possible errors from render_target
    */
   enum struct render_target_error_code
   {
      no_graphics_queue_found, ///< No graphics queue was found when creating a swapchain.
      no_present_queue_found   ///< No Present queue was found when creating a swapchain.
   };

   auto to_error_condition(render_target_error_code code) -> std::error_condition;

   class render_target
   {
   public:
      render_target() = default;
      render_target(vk::UniqueSurfaceKHR&& surface, monitor_dimensions const& dimensions,
                    spdlog::logger& logger);

      [[nodiscard]] auto surface() const noexcept -> vk::SurfaceKHR;

      void set_device(gfx::device&& device);
      void update_dimensions(monitor_dimensions const& dimensions);

   private:
      auto create_swapchain() -> vk::UniqueSwapchainKHR;

   private:
      target_status m_status = target_status::no_device;

      monitor_dimensions m_dimensions{};

      std::optional<gfx::device> m_device;

      vk::UniqueSurfaceKHR m_surface;
      vk::UniqueSwapchainKHR m_swapchain;

      [[maybe_unused]] spdlog::logger* mp_logger{};
   };
} // namespace owl::inline v0

#endif // LIBOWL_GFX_RENDER_TARGET_HPP_
