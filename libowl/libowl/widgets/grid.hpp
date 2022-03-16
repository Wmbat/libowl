/**
 * @file libowl/widgets/grid.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_WIDGETS_GRID_HPP_
#define LIBOWL_WIDGETS_GRID_HPP_

#include <libowl/widgets/widget.hpp>

#include <cassert>
#include <concepts>

namespace owl::inline v0
{
   namespace widget
   {
      class grid : public widget
      {
         using super = widget;

      public:
         grid(window& window, widget* p_parent);

         /*
         template <typename Widget, typename... Args>
            requires std::constructible_from<Widget, window&, widget*, Args...>
         auto make_widget(std::size_t row, std::size_t col, Args... args) -> Widget&
         {
            assert(is_gui_thread());
         }
         */

      private:
      };
   } // namespace widget
} // namespace owl::inline v0

#endif // LIBOWL_WIDGETS_GRID_HPP_
