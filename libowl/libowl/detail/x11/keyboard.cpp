#include <libowl/detail/x11/keyboard.hpp>

#include <libowl/detail/x11/keysym_to_code_point_table.hpp>

#include <libmannele/algorithm/binary_search.hpp>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include <span>

using reglisse::maybe;
using reglisse::none;
using reglisse::some;

namespace owl::inline v0
{
   namespace
   {
      using kbd_mapping_reply_ptr =
         std::unique_ptr<xcb_get_keyboard_mapping_reply_t, void (*)(void*)>;

      auto get_keyboard_mapping(xcb_connection_t* p_conn, const xcb_setup_t* p_setup)
         -> kbd_mapping_reply_ptr
      {
         const auto cookie = xcb_get_keyboard_mapping(
            p_conn, p_setup->min_keycode, p_setup->max_keycode - p_setup->min_keycode + 1);

         return {xcb_get_keyboard_mapping_reply(p_conn, cookie, nullptr), free};
      }

      auto compute_keysim_offset(kbd::modifier_flags flags) -> u32
      {
         const bool has_shift =
            (flags & kbd::modifier_flag_bits::shift) == kbd::modifier_flag_bits::shift;
         const bool has_caps_lock =
            (flags & kbd::modifier_flag_bits::caps_lock) == kbd::modifier_flag_bits::caps_lock;

         if (has_shift && has_caps_lock)
         {
            return 0;
         }
         if (has_shift || has_caps_lock)
         {
            return 1;
         }
         else
         {
            return 0;
         }
      }
   } // namespace

   namespace x11
   {
      auto key_press_state_mask_to_modifiers(u32 state_mask) -> kbd::modifier_flags
      {
         kbd::modifier_flags mods;

         if (state_mask & XCB_MOD_MASK_SHIFT)
         {
            mods |= kbd::modifier_flag_bits::shift;
         }

         if (state_mask & XCB_MOD_MASK_LOCK)
         {
            mods |= kbd::modifier_flag_bits::caps_lock;
         }

         if (state_mask & XCB_MOD_MASK_CONTROL)
         {
            mods |= kbd::modifier_flag_bits::ctrl;
         }

         return mods;
      }

      auto find_correct_keysym(const unique_connection& connection, keycode_t key_code,
                               kbd::modifier_flags modifiers) -> keysym_t
      {
         const xcb_setup_t* p_setup = xcb_get_setup(connection.get());
         const auto kbd_mapping = get_keyboard_mapping(connection.get(), p_setup);

         const i32 keysym_count = xcb_get_keyboard_mapping_keysyms_length(kbd_mapping.get());
         const xcb_keysym_t* p_keysyms = xcb_get_keyboard_mapping_keysyms(kbd_mapping.get());

         const auto keysyms = std::span(p_keysyms, keysym_count);

         const auto keysyms_per_key_code = kbd_mapping->keysyms_per_keycode;
         const auto adjusted_key_code = key_code.value() - p_setup->min_keycode;
         const auto keysym_offset = compute_keysim_offset(modifiers);

         return keysym_t(keysyms[keysym_offset + adjusted_key_code * keysyms_per_key_code]);
      }

      auto keysym_to_ucs(keysym_t keysym) -> maybe<detail::code_point_t>
      {
         const auto it = mannele::binary_search(keysym_to_UCS_table, keysym, std::ranges::less(),
                                                [](const auto& p) { return p.keysym; });

         if (it != std::ranges::end(keysym_to_UCS_table))
         {
            return some(it->code_point);
         }
         else
         {
            return none;
         }
      }
   } // namespace x11
} // namespace owl::inline v0
