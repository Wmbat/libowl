/**
 *
 */

#ifndef LIBOWL_GUI_KEYBOARD_EVENT_HPP_
#define LIBOWL_GUI_KEYBOARD_EVENT_HPP_

#include <libowl/gui/keyboard_modifiers.hpp>

#include <chrono>

namespace owl::inline v0
{
   /**
    *
    */
   enum struct key_event_type
   {
      press,  ///<
      release ///<
   };

   /**
    *
    */
   struct key_event
   {
      key_event_type type;            ///<
      key_modifier_flags mods;        ///<
      std::chrono::milliseconds time; ///<
   };
} // namespace owl::inline v0

#endif // LIBOWL_GUI_KEYBOARD_EVENT_HPP_
