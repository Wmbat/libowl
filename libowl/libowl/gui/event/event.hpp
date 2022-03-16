/**
 * @file libowl/gui/event/event.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief Contains everything related to event handling in libowl
 * @copyright Copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_GUI_EVENT_HPP_
#define LIBOWL_GUI_EVENT_HPP_

#if defined(LIBOWL_USE_X11)
#   include <libowl/detail/x11/connection.hpp>

#   include <xcb/xcb.h>
#endif // defined (LIBOWL_USE_X11)

#include <libowl/gui/event/command.hpp>
#include <libowl/gui/event/focus_event.hpp>
#include <libowl/gui/event/keyboard_event.hpp>
#include <libowl/gui/event/mouse_event.hpp>
#include <libowl/gui/event/structure_changed_event.hpp>

#include <variant>
#include <optional>

namespace owl::inline v0
{
   /**
    * @brief type alias for a union over all supported event types
    */
   using event_variant = std::variant<key_event, mouse_button_event, mouse_movement_event,
                                      structure_changed_event, focus_event, command>;

#if defined(LIBOWL_USE_X11)
   /**
    * @brief Check if there are any new events to handle
    *
    * @param[in] conn The connection to the X server
    *
    * @return the maybe will be empty if there is no event
    */
   auto poll_for_event(x11::connection const& conn) -> std::optional<event_variant>;
#endif // defined (LIBOWL_USE_X11)
} // namespace owl::inline v0

#endif // LIBOWL_GUI_EVENT_HPP_
