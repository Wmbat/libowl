/**
 * @file libowl/widgets/window.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#include <libowl/widgets/window.hpp>

#include <libowl/widgets/label.hpp>

namespace owl::inline v0
{
   namespace widget
   {
      window::window(owl::window& wnd) :
         widget(wnd, nullptr), m_titlebar(wnd, this), m_frame(wnd, this)
      {}

      auto window::frame() noexcept -> grid& { return m_frame; }
   } // namespace widget
} // namespace owl::inline v0
