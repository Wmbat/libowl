#include <libowl/gui/event.hpp>

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
      using unique_event = std::unique_ptr<xcb_generic_event_t, void (*)(void *)>;

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
   } // namespace

   auto poll_for_event(const x11::connection &conn) -> maybe<event_variant>
   {
      const auto event = unique_event(xcb_poll_for_event(conn.x_server.get()), free);

      if (event)
      {
         const auto event_type = event->response_type & ~0x80;

         if (event_type == XCB_KEY_PRESS)
         {
            // NOLINTNEXTLINE
            auto *key_press_event = reinterpret_cast<xcb_key_press_event_t *>(event.get());
            const auto state = key_press_event->state;

            const auto modifiers = key_press_state_mask_to_modifiers(state);
            const auto keycode = x11::keycode_t(key_press_event->detail);
            const auto keysym = x11::keysym_t(conn, keycode, modifiers);
            const auto ucs_value = x11::to_code_point(keysym);

            return some(event_variant(key_event{
               .mods = modifiers, .time = std::chrono::milliseconds(key_press_event->time)}));
         }
         else if (event_type == XCB_CLIENT_MESSAGE)
         {
            // NOLINTNEXTLINE
            auto *client_message = reinterpret_cast<xcb_client_message_event_t *>(event.get());

            fmt::print("CLIENT_MESSAGE\n{{\n\t.type = {},\n\t.format = {}\n}}",
                       client_message->type, client_message->format);

            return none;
         }
         else
         {

            return none;
         }
      }
      else
      {
         return none;
      }
   }
#endif // defined(LIBOWL_USE_X11)
} // namespace owl::inline v0
