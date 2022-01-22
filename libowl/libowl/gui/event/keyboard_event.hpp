/**
 * @file libowl/gui/event/keyboard_event.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief 
 * @copyright Copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_GUI_KEYBOARD_EVENT_HPP_
#define LIBOWL_GUI_KEYBOARD_EVENT_HPP_

#include <libowl/gui/keyboard_modifiers.hpp>

// C++ Standard Library includes

#include <chrono>

namespace owl::inline v0
{
   /**
    * @brief Represents all possible key event types
    */
   enum struct key_event_type
   {
      press,  ///< 
      release ///<
   };

   /**
    * @brief
    */
   struct key_event
   {
      key_event_type type;            ///< The type of key event
      key_modifier_flags mods;        ///< The key modifiers of the event
      std::chrono::milliseconds time; ///< The time in when the key was pressed
   };
} // namespace owl::inline v0

#endif // LIBOWL_GUI_KEYBOARD_EVENT_HPP_
