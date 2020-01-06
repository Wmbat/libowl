/*
 *  Copyright (C) 2018-2019 Wmbat
 *
 *  wmbat@protonmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  You should have received a copy of the GNU General Public License
 *  GNU General Public License for more details.
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUCIOLE_WINDOW_KEYBOARD_HPP
#define LUCIOLE_WINDOW_KEYBOARD_HPP

#include <cstdint>

/*!
 * @brief Hold the keymapping for keyboard keys.
 */
class keyboard
{
public:
   enum class key_state : std::int32_t
   {
      invalid = -1,
      pressed = 0,
      released = 1
   };

   enum class key : std::int32_t
   {
      invalid = -1,
      nun_1 = 10,
      num_2 = 11,
      num_3 = 12,
      num_4 = 13,
      num_5 = 14,
      num_6 = 15,
      num_7 = 16,
      num_8 = 17,
      num_9 = 18,
      num_0 = 19,
      f1 = 67,
      f2 = 68,
      f3 = 69,
      f4 = 70,
      f5 = 71,
      f6 = 72,
      f7 = 73,
      f8 = 74,
      f9 = 75,
      f10 = 76,
      f11 = 95,
      f12 = 96,
      numpad_1 = 87,
      numpad_2 = 88,
      numpad_3 = 89,
      numpad_4 = 83,
      numpad_5 = 84,
      numpad_6 = 85,
      numpad_7 = 79,
      numpad_8 = 80,
      numpad_9 = 81,
      numpad_0 = 90,
      numpad_enter = 104,
      numpad_plus = 86,
      numpad_minus = 82,
      numpad_star = 63,
      numpad_backslash = 106,
      numpad_period = 91,
      a = 38,
      b = 57,
      c = 31,
      d = 44,
      e = 25,
      f = 29,
      g = 30,
      h = 44,
      i = 42,
      j = 54,
      k = 55,
      l = 33,
      m = 58,
      n = 46,
      o = 39,
      p = 27,
      q = 53,
      r = 32,
      s = 47,
      t = 45,
      u = 41,
      v = 60,
      w = 59,
      x = 56,
      y = 28,
      z = 61,
      tilde = 49,
      single_quote = 24,
      dual_quotes = 24,
      l_arrow_bracket = 25,
      r_arrow_brakect = 26,
      comma = 25,
      period = 26,
      l_square_bracket = 20,
      r_square_bracket = 21,
      l_curly_bracket = 20,
      r_curly_bracket = 21,
      back_slash = 34,
      question_mark = 34,
      equal = 35,
      plus = 35,
      front_slash = 51,
      pipe = 51,
      minus = 48,
      under_score = 48,
      spacebar = 65,
      backspace = 22,
      enter = 36,
      tab = 23,
      caps_lock = 66,
      l_shift = 50,
      l_ctrl = 37,
      l_alt = 64,
      r_shift = 62,
      r_ctrl = 105,
      r_alt = 108,
      up_arrow = 111,
      down_arrow = 116,
      left_arrow = 113,
      right_arrow = 114,
      last = 256
   };
};

#endif // LUCIOLE_WINDOW_KEYBOARD_HPP
