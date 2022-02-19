/**
 * @file libowl/gui/event/event.cpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief
 * @copyright Copyright (C) 2022 wmbat
 */

#include <libowl/gui/event/event.hpp>

#include <libowl/gui/keyboard_modifiers.hpp>

#if defined(LIBOWL_USE_X11)
#   include <libowl/detail/x11/keycode.hpp>
#   include <libowl/detail/x11/keysym.hpp>

#   include <xcb/xcb.h>
#endif // defined(LIBOWL_USE_X11)

using reglisse::maybe;
using reglisse::none;
using reglisse::some;

namespace owl::inline v0
{
#if defined(LIBOWL_USE_X11)
   namespace
   {
      using unique_event = std::unique_ptr<xcb_generic_event_t, void (*)(void*)>;

      /**
       * @brief Convert an xcb state mask to a key_modifier_flags
       */
      auto key_press_state_mask_to_modifiers(u32 state_mask) -> key_modifier_flags
      {
         key_modifier_flags mods;

         if (state_mask & XCB_MOD_MASK_SHIFT)
         {
            mods |= key_modifiers_flag_bits::shift;
         }

         if (state_mask & XCB_MOD_MASK_LOCK)
         {
            mods |= key_modifiers_flag_bits::caps_lock;
         }

         if (state_mask & XCB_MOD_MASK_CONTROL)
         {
            mods |= key_modifiers_flag_bits::ctrl;
         }

         return mods;
      }

      auto handle_key_event(x11::connection const& conn, xcb_key_press_event_t const& event,
                            key_event_type type) -> key_event
      {
         auto const state = event.state;

         auto const modifiers = key_press_state_mask_to_modifiers(state);
         auto const keycode = x11::keycode_t(event.detail);
         auto const keysym = x11::keysym_t(conn, keycode, modifiers);
         auto const ucs_value = x11::to_code_point(keysym);

         return {.type = type, .mods = modifiers, .time = std::chrono::milliseconds(event.time)};
      }

      auto handle_configure_notify_event(xcb_configure_notify_event_t const& event)
         -> structure_changed_event
      {
         return {
            .dimension = {.x = event.x, .y = event.y, .width = event.width, .height = event.height},
            .window_id = event.window};
      }

      template <typename T>
      auto to_event_type(unique_event const& event) -> T const&
      {
         return *reinterpret_cast<T const*>(event.get()); // NOLINT
      }

   } // namespace

   auto poll_for_event(x11::connection const& conn) -> maybe<event_variant>
   {
      if (auto const event = unique_event(xcb_poll_for_event(conn.x_server.get()), free))
      {
         auto const event_type = event->response_type & ~0x80;

         if (event_type == XCB_KEY_PRESS)
         {
            auto const& key_press_event = to_event_type<xcb_key_press_event_t>(event);
            auto const event = handle_key_event(conn, key_press_event, key_event_type::press);

            return some(event_variant(event));
         }
         else if (event_type == XCB_KEY_RELEASE)
         {
            auto const& key_release_event = to_event_type<xcb_key_release_event_t>(event);
            auto const event = handle_key_event(conn, key_release_event, key_event_type::release);

            return some(event_variant(event));
         }
         else if (event_type == XCB_MOTION_NOTIFY)
         {
            return some(event_variant(command::ignore));
         }
         else if (event_type == XCB_ENTER_NOTIFY)
         {
            return some(event_variant(command::ignore));
         }
         else if (event_type == XCB_LEAVE_NOTIFY)
         {
            return some(event_variant(command::ignore));
         }
         else if (event_type == XCB_FOCUS_IN)
         {
            auto const& focus_in_event = to_event_type<xcb_focus_in_event_t>(event);

            return some(event_variant(
               focus_event{.type = focus_type::in, .window_id = focus_in_event.event}));
         }
         else if (event_type == XCB_FOCUS_OUT)
         {
            auto const& focus_out_event = to_event_type<xcb_focus_out_event_t>(event);

            return some(event_variant(
               focus_event{.type = focus_type::out, .window_id = focus_out_event.event}));
         }
         else if (event_type == XCB_EXPOSE)
         {
            return some(event_variant(command::render_window));
         }
         else if (event_type == XCB_CONFIGURE_NOTIFY)
         {
            auto const& configure_notify_event = to_event_type<xcb_configure_notify_event_t>(event);

            return some(event_variant(handle_configure_notify_event(configure_notify_event)));
         }
         else if (event_type == XCB_CLIENT_MESSAGE)
         {
            auto const client_message = to_event_type<xcb_client_message_event_t>(event);
            if (client_message.type == conn.protocol_prop.atom)
            {
               if (client_message.data.data32[0] == conn.protocol_prop.delete_atom)
               {
                  return some(event_variant(command::close_window));
               }
               else
               {
                  return some(event_variant(command::ignore));
               }
            }

            return some(event_variant(command::ignore));
         }
         else
         {
            return some(event_variant(command::ignore));
         }
      }
      else
      {
         return none;
      }
   }
#endif // defined(LIBOWL_USE_X11)
} // namespace owl::inline v0
