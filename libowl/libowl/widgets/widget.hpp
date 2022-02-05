/**
 * @file libowl/widgets/widget.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#include <libowl/window.hpp>

namespace owl::inline v0
{
   class widget
   {
   public:
      widget(window& window, widget* p_parent);
  
      [[nodiscard]] auto is_gui_thread() const noexcept -> bool;

//      virtual void render() = 0;

      // Getters

      auto parent() -> const widget&;

   private:
      window& m_window;

      widget* mp_parent = nullptr;
   };
} // namespace owl::inline v0
