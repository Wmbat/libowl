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

#ifndef LUCIOLE_WINDOW_MOUSE_HPP
#define LUCIOLE_WINDOW_MOUSE_HPP

#include <cstdint>

/**
 *  @brief Holds the key mapping of a mouse button layout
 *  as well as the handling of the mouse button events.
 */
class mouse
{
public:
   enum class button_state : std::int32_t
   {
      invalid = -1,
      pressed = 0,
      released = 1
   };

   enum class button : std::int32_t
   {
      invalid = -1,
      l_button = 1,
      scroll_button = 2,
      r_button = 3,
      scroll_up = 4,
      scroll_down = 5,
      scroll_left = 6,
      scroll_right = 7,
      side_button_1 = 8,
      side_button_2 = 9,
      last = 16
   };
};

#endif // LUCIOLE_WINDOW_MOUSE_HPP
