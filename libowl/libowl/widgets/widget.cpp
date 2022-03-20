/**
 * @file libowl/widgets/widget.cpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#include <libowl/widgets/widget.hpp>

#include <libowl/window.hpp>

namespace owl::inline v0
{
   namespace widget
   {
      widget::widget(owl::window& window, widget* p_parent) : m_window(window), mp_parent(p_parent)
      {
         //      assert(is_gui_thread()); // NOLINT
      }

      [[nodiscard]] auto widget::is_gui_thread() const noexcept -> bool
      {
         return m_window.is_gui_thread();
      }

      [[nodiscard]] auto widget::owning_window() const noexcept -> owl::window const&
      {
         return m_window;
      }
      auto widget::owning_window() noexcept -> owl::window& { return m_window; }
   } // namespace widget
} // namespace owl::inline v0
