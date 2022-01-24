/**
 * @file libowl/widgets/widget.cpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#include <libowl/widgets/widget.hpp>

#include <libgerbil/assert.hpp>

namespace owl::inline v0
{
   widget::widget(window& window, widget* p_parent) : m_window(window), mp_parent(p_parent)
   {
      assert(is_gui_thread()); // NOLINT
   }

   [[nodiscard]] auto widget::is_gui_thread() const noexcept -> bool
   {
      return m_window.is_gui_thread();
   }
} // namespace owl::inline v0
