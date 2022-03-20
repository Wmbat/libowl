/**
 * @file libowl/window.cpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#include <libowl/window.hpp>

#include <libowl/system.hpp>

namespace owl::inline v0
{
   void window::render(std::chrono::nanoseconds) {}

   void window::handle_event(const key_event&) {}

   [[nodiscard]] auto window::is_gui_thread() const noexcept -> bool
   {
      return m_system.is_gui_thread();
   }

   [[nodiscard]] auto window::title() const noexcept -> std::string_view { return m_title; }
   [[nodiscard]] auto window::target() const noexcept -> const render_target&
   {
      return m_render_target;
   }
   [[nodiscard]] auto window::monitor() const noexcept -> const owl::monitor&
   {
      assert(mp_target_monitor != nullptr); // NOLINT

      return *mp_target_monitor;
   }

   void window::set_device(gfx::device&& device) noexcept
   {
      m_render_target.set_device(std::move(device));
   }

   window::window(system& system, std::string_view title, owl::monitor& target_monitor,
                  spdlog::logger& logger) :
      m_system(system),
      m_logger(logger), m_title(title), mp_target_monitor(&target_monitor)
   {}

   [[nodiscard]] auto window::logger() const noexcept -> spdlog::logger& { return m_logger; }

   void window::set_render_target(render_target&& target) { m_render_target = std::move(target); }
} // namespace owl::inline v0
