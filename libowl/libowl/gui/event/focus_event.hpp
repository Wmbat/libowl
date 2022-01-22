/**
 * @file libowl/gui/event/focus_event.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief Holds elements related to window focus events
 * @copyright Copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_GUI_EVENT_FOCUS_EVENT_HPP_
#define LIBOWL_GUI_EVENT_FOCUS_EVENT_HPP_

#include <libowl/types.hpp>

namespace owl::inline v0
{
   /**
    * @brief Represent all possible focus types
    */
   enum struct focus_type
   {
      in, ///< Signifies a window gaining focus
      out ///< Signifies a window loosing focus
   };

   /**
    * @brief Event to let the system know which window is in focus or not
    */
   struct focus_event
   {
      focus_type type; ///< The type of focus
      u32 window_id;   ///< The window who's focus status changed
   };
} // namespace owl::inline v0

#endif // LIBOWL_GUI_EVENT_FOCUS_EVENT_HPP_
