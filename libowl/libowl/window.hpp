/**
 * @file libowl/window.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBOWL_WINDOW_HPP_
#define LIBOWL_WINDOW_HPP_

#include <libowl/gfx/device.hpp>
#include <libowl/gfx/render_target.hpp>
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

      void set_device(gfx::device&& device) noexcept;

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
      [[nodiscard]] auto target() const noexcept -> render_target const&;
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

      void set_render_target(render_target&& target);

   private:
      system& m_system;

      spdlog::logger& m_logger;

      std::string m_title;

      owl::monitor* mp_target_monitor;

      render_target m_render_target;
   };

   using unique_window = std::unique_ptr<window>;
} // namespace owl::inline v0

#endif // LIBOWL_WINDOW_HPP_
