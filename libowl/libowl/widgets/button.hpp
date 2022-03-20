/**
 * @file libowl/widgets/button.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_WIDGETS_BUTTON_HPP_
#define LIBOWL_WIDGETS_BUTTON_HPP_

#include <libowl/widgets/widget.hpp>

namespace owl::inline v0
{
   namespace widget
   {
      class button : public widget
      {
         using super = widget;

      public:
         button(window& window, widget* p_parent);

      private:
      };
   } // namespace widget
} // namespace owl::inline v0

#endif // LIBOWL_WIDGETS_BUTTON_HPP_
