/**
 *
 */

#ifndef LIBOWL_GUI_KEYBOARD_EVENT_HPP_
#define LIBOWL_GUI_KEYBOARD_EVENT_HPP_

#include <libowl/gui/keyboard_modifiers.hpp>

#include <chrono>

namespace owl::inline v0
{
   struct key_event
   {
      key_modifier_flags mods;        ///<
      std::chrono::milliseconds time; ///<
   };
} // namespace owl::inline v0

#endif // LIBOWL_GUI_KEYBOARD_EVENT_HPP_
