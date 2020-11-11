#pragma once

#include <vkn/core.hpp>

#include <GLFW/glfw3.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace gfx
{
   class window
   {
      using wnd_ptr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;

   public:
      window();
      window(std::string_view title_in, std::uint32_t width_in, std::uint32_t height_in);

      void poll_events();

      auto is_open() -> bool;

      [[nodiscard]] auto get_surface(vk::Instance instance) const -> util::result<vk::SurfaceKHR>;

   private:
      std::string title{"EGL standard window"};
      std::uint32_t width{1080u};
      std::uint32_t height{720u};

      wnd_ptr p_wnd;
   };
} // namespace gfx
