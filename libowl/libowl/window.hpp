/**
 * @file libowl/window.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBOWL_WINDOW_HPP_
#define LIBOWL_WINDOW_HPP_

#include <libowl/gfx/render_surface.hpp>
#include <libowl/gui/event/keyboard_event.hpp>
#include <libowl/gui/monitor.hpp>

#include <libash/device.hpp>

// C++ Standard Library

#include <functional>
#include <memory>

namespace owl::inline v0
{
   class system;

   /**
    * @brief
    */
   class window
   {
   public:
      window(window const& other) = delete;
      window(window&& other) noexcept = delete;
      virtual ~window() = default;

      auto operator=(window const& other) = delete;
      auto operator=(window&& other) noexcept = delete;

      virtual void render(std::chrono::nanoseconds delta_time);

      void handle_event(key_event const& event);

      /**
       * @brief Set the window's physical device used for rendering.
       *
       * @param[in] device The physical device.
       */
      void set_physical_device(ash::physical_device&& device) noexcept;

      /**
       * @brief
       */
      [[nodiscard]] auto is_gui_thread() const noexcept -> bool;

      /**
       * @brief Get the window title
       */
      [[nodiscard]] auto title() const noexcept -> std::string_view;
      /**
       * @brief Get the window's render surface
       */
      [[nodiscard]] auto surface() const noexcept -> render_surface const&;
      /**
       * @brief Get the monitor the window currently is on
       */
      [[nodiscard]] auto monitor() const noexcept -> owl::monitor const&;
      /**
       * @brief Get the id of the window
       */
      [[nodiscard]] virtual auto id() const noexcept -> u32 = 0;

   protected:
      window(system& system, std::string_view title, owl::monitor& target_monitor,
             spdlog::logger& logger);

      /**
       * @brief Get the logger used by the window
       */
      [[nodiscard]] auto logger() const noexcept -> spdlog::logger&;

      void set_surface(render_surface&& surface);

   private:
      system& m_system;

      spdlog::logger& m_logger;

      std::string m_title;

      owl::monitor* mp_target_monitor;

      render_surface m_surface;
      ash::physical_device m_physical_device;
      ash::device m_device;
   };

   using unique_window = std::unique_ptr<window>;
} // namespace owl::inline v0

#endif // LIBOWL_WINDOW_HPP_
