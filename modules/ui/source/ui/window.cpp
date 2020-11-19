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

#include <ui/window.hpp>

#include <memory>
#include <system_error>

namespace ui
{
   window::window(std::string_view title_in, std::uint32_t width_in, std::uint32_t height_in) :
      title(title_in), width(width_in), height(height_in)
   {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

      p_wnd = wnd_ptr(glfwCreateWindow(static_cast<int>(width), static_cast<int>(height),
                                       title.c_str(), nullptr, nullptr),
                      glfwDestroyWindow);
   }

   void window::poll_events() { glfwPollEvents(); }

   auto window::is_open() -> bool { return !glfwWindowShouldClose(p_wnd.get()); }

   auto window::get_surface(vk::Instance instance) const -> result<vk::UniqueSurfaceKHR>
   {
      VkSurfaceKHR surface = VK_NULL_HANDLE;
      const auto res = glfwCreateWindowSurface(instance, p_wnd.get(), nullptr, &surface);

      if (res != VK_SUCCESS)
      {
         return monad::err(to_error_t(window_error::failed_to_create_surface));
      }

      return vk::UniqueSurfaceKHR{surface, instance};
   }

   struct window_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override { return "ui_window"; }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<window_error>(err));
      }
   };

   static const window_error_category window_error_cat{};

   auto to_string(window_error err) -> std::string
   {
      if (err == window_error::failed_to_create_surface)
      {
         return "failed_to_create_surface";
      }
      else
      {
         return "UNKNOWN";
      }
   }
   auto to_error_t(window_error err) -> error_t
   {
      return {{static_cast<int>(err), window_error_cat}};
   }
} // namespace ui
