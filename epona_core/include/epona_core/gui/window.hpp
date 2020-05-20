/**
 * MIT License
 *
 * Copyright (c) 2020 Wmbat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "epona_core/gui/widget.hpp"
#include "epona_core/vk/core.hpp"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace EGL
{
   class window : public widget
   {
      using wnd_ptr = std::unique_ptr<GLFWwindow, std::function<void( GLFWwindow* )>>;

   public:
      window( );
      window( std::string_view title_in, std::uint32_t width_in, std::uint32_t height_in );

      bool is_open( );

   private:
      std::string title{ "EGL standard window" };
      std::uint32_t width{ 1080u };
      std::uint32_t height{ 720u };

      wnd_ptr p_wnd;
   };
} // namespace EGL
