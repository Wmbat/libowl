/**
 * @file libowl/widgets/label.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#ifndef LIBOWL_WIDGETS_LABEL_HPP_
#define LIBOWL_WIDGETS_LABEL_HPP_

#include <libowl/widgets/widget.hpp>

namespace owl::inline v0
{
   namespace widget
   {
      class label : widget
      {
      public:
         label(window& wnd, widget* p_parent);

      private:
      };
   } // namespace widget
} // namespace owl::inline v0

#endif // LIBOWL_WIDGETS_LABEL_HPP_
