#ifndef LIBOWL_GUI_EVENT_HPP_
#define LIBOWL_GUI_EVENT_HPP_

#if defined(LIBOWL_USE_X11)
#   include <libowl/detail/x11/connection.hpp>
#endif // defined (LIBOWL_USE_X11)

#include <libowl/gui/command.hpp>
#include <libowl/gui/keyboard_event.hpp>
#include <libowl/gui/mouse_event.hpp>

#include <libreglisse/maybe.hpp>

#include <variant>

namespace owl::inline v0
{
   using event_variant = std::variant<key_event, mouse_button_event, mouse_movement_event, command>;

#if defined(LIBOWL_USE_X11)
   /**
    * @brief
    *
    * @param[in] conn
    */
   auto poll_for_event(const x11::connection &conn) -> reglisse::maybe<event_variant>;
#endif // defined (LIBOWL_USE_X11)
} // namespace owl::inline v0

#endif // LIBOWL_GUI_EVENT_HPP_
