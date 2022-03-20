/**
 * @file libowl/widgets/widget.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#ifndef LIBOWL_WIDGETS_WIDGET_HPP_
#define LIBOWL_WIDGETS_WIDGET_HPP_

namespace owl::inline v0
{
   class window;

   namespace widget
   {
      class widget
      {
      public:
         widget(owl::window& window, widget* p_parent);

         [[nodiscard]] auto is_gui_thread() const noexcept -> bool;

         //      virtual void render() = 0;

         // Getters

         // [[nodiscard]] auto parent() const noexcept -> widget const&;
         [[nodiscard]] auto owning_window() const noexcept -> window const&;
         auto owning_window() noexcept -> window&;

      private:
         owl::window& m_window;

         widget* mp_parent = nullptr;
      };
   } // namespace widget
} // namespace owl::inline v0

#endif // LIBOWL_WIDGETS_WIDGET_HPP_
