/**
 * @file libowl/widgets/window.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#ifndef LIBOWL_WIDGETS_WINDOW_HPP_
#define LIBOWL_WIDGETS_WINDOW_HPP_

#include <libowl/widgets/grid.hpp>

namespace owl::inline v0
{
   namespace widget
   {
      class window : widget
      {
      public:
         window(owl::window& wnd);

         auto frame() noexcept -> grid&;

      private:
         grid m_titlebar;

         grid m_frame;
      };
   } // namespace widget
} // namespace owl::inline v0

#endif // LIBOWL_WIDGETS_WINDOW_HPP_
