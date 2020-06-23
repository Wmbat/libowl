#pragma once

#include "epona_core/graphics/gui/widget.hpp"
#include "epona_core/graphics/vkn/core.hpp"

#include <GLFW/glfw3.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace core::gfx
{
   class window : public widget
   {
      using wnd_ptr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;

   public:
      window();
      window(std::string_view title_in, std::uint32_t width_in, std::uint32_t height_in);

      bool is_open();

      vkn::result<vk::UniqueSurfaceKHR> get_surface(vk::Instance instance) const;

   private:
      std::string title{"EGL standard window"};
      std::uint32_t width{1080u};
      std::uint32_t height{720u};

      wnd_ptr p_wnd;
   };
} // namespace core::gfx
