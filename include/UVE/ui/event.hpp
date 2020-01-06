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

#pragma once

#include <UVE/ui/keyboard.hpp>
#include <UVE/ui/mouse.hpp>
#include <UVE/utils/delegate.hpp>

#include <glm/glm.hpp>

struct key_event
{
   keyboard::key code;
   keyboard::key_state state;
};

struct mouse_button_event
{
   mouse::button code;
   mouse::button_state state;
   glm::i32vec2 position;
};

struct mouse_motion_event
{
   glm::i32vec2 position;
};

struct window_close_event
{
   bool is_closed;
};

struct framebuffer_resize_event
{
   glm::u32vec2 size;
};

using key_event_delg = delegate<void( const key_event& )>;
using mouse_button_event_delg = delegate<void( const mouse_button_event& )>;
using mouse_motion_event_delg = delegate<void( const mouse_motion_event& )>;
using window_close_event_delg = delegate<void( const window_close_event& )>;
using framebuffer_resize_event_delg = delegate<void( const framebuffer_resize_event& )>;
