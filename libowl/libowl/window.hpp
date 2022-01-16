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
      window(const window& other) = delete;
      window(window&& other) noexcept = delete;
      virtual ~window() = default;

      auto operator=(const window& other) = delete;
      auto operator=(window&& other) noexcept = delete;

      virtual void render(std::chrono::nanoseconds delta_time);

      [[nodiscard]] virtual auto id() const noexcept -> u32 = 0;

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
      [[nodiscard]] auto surface() const noexcept -> const render_surface&;

   protected:
      window(system& system, std::string_view title, spdlog::logger& logger);

      [[nodiscard]] auto logger() const noexcept -> spdlog::logger&;

      void set_surface(render_surface&& surface);

   private:
      system& m_system;

      spdlog::logger& m_logger;

      std::string m_title;

      render_surface m_surface;
      ash::physical_device m_physical_device;
      ash::device m_device;
   };

   using unique_window = std::unique_ptr<window>;
} // namespace owl::inline v0

#endif // LIBOWL_WINDOW_HPP_
