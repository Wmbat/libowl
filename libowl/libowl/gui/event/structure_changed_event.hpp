/**
 * @file libowl/gui/event/structure_changed_event.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief 
 * @copyright Copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_GUI_EVENT_STRUCTURE_CHANGED_EVENT_HPP_
#define LIBOWL_GUI_EVENT_STRUCTURE_CHANGED_EVENT_HPP_

#include <libowl/gui/monitor.hpp>
#include <libowl/types.hpp>

namespace owl::inline v0
{
   /**
    *
    */
   struct structure_changed_event
   {
      monitor_dimensions dimension; ///< The new dimensions of the window in pixels
      u32 window_id;                ///< The id of the window that was modified
   };
} // namespace owl::inline v0

#endif // LIBOWL_GUI_EVENT_STRUCTURE_CHANGED_EVENT_HPP_
