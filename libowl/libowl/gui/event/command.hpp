/**
 * @file libowl/gui/event/command.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief 
 * @copyright Copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_GUI_COMMAND_HPP_
#define LIBOWL_GUI_COMMAND_HPP_

namespace owl::inline v0
{
   /**
    * @brief
    */
   enum struct command
   {
      render_window, ///< Orders a redraw of the gui
      close_window, ///< Orders a window to be closed
      ignore
   };
}

#endif // LIBOWL_GUI_COMMAND_HPP_
