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
      /**
       * @brief
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

      auto handle_key_event(const x11::connection &conn, const xcb_key_press_event_t &event,
                            key_event_type type) -> key_event
      {
         const auto state = event.state;

         const auto modifiers = key_press_state_mask_to_modifiers(state);
         const auto keycode = x11::keycode_t(event.detail);
         const auto keysym = x11::keysym_t(conn, keycode, modifiers);
         const auto ucs_value = x11::to_code_point(keysym);

         return {.type = type, .mods = modifiers, .time = std::chrono::milliseconds(event.time)};
      }

      template <typename T>
      auto to_event_type(const unique_event &event) -> const T &
      {
         return *reinterpret_cast<const T *>(event.get()); // NOLINTNEXTLINE
      }
   } // namespace

   auto poll_for_event(const x11::connection &conn) -> maybe<event_variant>
   {
      if (const auto event = unique_event(xcb_poll_for_event(conn.x_server.get()), free))
      {
         const auto event_type = event->response_type & ~0x80;

         if (event_type == XCB_KEY_PRESS)
         {
            const auto key_press_event = to_event_type<xcb_key_press_event_t>(event);
            const auto event = handle_key_event(conn, key_press_event, key_event_type::press);

            return some(event_variant(event));
         }
         else if (event_type == XCB_KEY_RELEASE)
         {
            const auto key_release_event = to_event_type<xcb_key_release_event_t>(event);
            const auto event = handle_key_event(conn, key_release_event, key_event_type::release);

            return some(event_variant(event));
         }
         else if (event_type == XCB_FOCUS_IN)
         {
            const auto focus_in_event = to_event_type<xcb_focus_in_event_t>(event);

            return some(event_variant(
               focus_event{.type = focus_type::in, .window_id = focus_in_event.event}));
         }
         else if (event_type == XCB_FOCUS_OUT)
         {
            const auto focus_out_event = to_event_type<xcb_focus_out_event_t>(event);

            return some(event_variant(
               focus_event{.type = focus_type::out, .window_id = focus_out_event.event}));
         }
         else if (event_type == XCB_ENTER_NOTIFY)
         {
            fmt::print("mouse entered window");
            return some(event_variant(command::ignore));
         }
         else if (event_type == XCB_LEAVE_NOTIFY)
         {
            fmt::print("mouse left window");
            return some(event_variant(command::ignore));
         }
         else if (event_type == XCB_EXPOSE)
         {
            return some(event_variant(command::render_window));
         }
         else if (event_type == XCB_CLIENT_MESSAGE)
         {
            // NOLINTNEXTLINE
            auto *client_message = reinterpret_cast<xcb_client_message_event_t *>(event.get());
            if (client_message->type == conn.window_protocol_atom
                and client_message->data.data32[0] == conn.window_delete_atom)
            {
               return some(event_variant(command::close_window));
            }

            return some(event_variant(command::ignore));
         }
         else
         {
            return some(event_variant(command::close_window));
         }
      }
      else
      {
         return none;
      }
   }
#endif // defined(LIBOWL_USE_X11)
} // namespace owl::inline v0
