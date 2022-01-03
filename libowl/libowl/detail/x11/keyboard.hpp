/**
 * @file libowl/detail/x11/keyboard.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#ifndef LIBOWL_DETAIL_X11_X11_KEYBOARD_HPP_
#define LIBOWL_DETAIL_X11_X11_KEYBOARD_HPP_

#include <libowl/detail/keyboard/code_point.hpp>
#include <libowl/detail/x11/types.hpp>
#include <libowl/gui/keyboard_modifiers.hpp>
#include <libowl/types.hpp>
#include <libowl/window/x11_support.hpp>

#include <libreglisse/maybe.hpp>

#include <xcb/xcb_keysyms.h>

#include <memory>

namespace owl::inline v0
{
   namespace x11
   {
      /**
       * @brief 
       */
      auto key_press_state_mask_to_modifiers(u32 state_mask) -> kbd::modifier_flags;

      /**
       * @brief Finds the correct keysym for a keycode using keyboard modifiers
       *
       * @param[in] connection The connection to the X server
       * @param[in] key_code The X11 key_code received form a key press event
       * @param[in] modifiers The keyboard modifiers to apply on the key_code
       *
       * @return The keysym corresponding to the key_code and modifiers
       */
      auto find_correct_keysym(const unique_connection& connection, keycode_t key_code,
                               kbd::modifier_flags modifiers) -> keysym_t;

      /**
       * @brief
       *
       * @param[in] keysym
       *
       * @return
       */
      auto keysym_to_ucs(keysym_t keysym) -> reglisse::maybe<detail::code_point_t>;
   } // namespace x11
} // namespace owl::inline v0

#endif // LIBOWL_DETAIL_X11_X11_KEYBOARD_HPP_
