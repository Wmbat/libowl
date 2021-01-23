#pragma once

#include <cacao/ui/core.hpp>

#include <cacao/context.hpp>

#include <GLFW/glfw3.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace ui
{
   enum struct window_error
   {
      failed_to_create_surface
   };

   auto to_string(window_error err) -> std::string;
   auto to_error_t(window_error err) -> error_t;

   class window
   {
      using wnd_ptr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;

   public:
      window(std::string_view title_in, std::uint32_t width_in, std::uint32_t height_in);

      void poll_events();

      auto is_open() -> bool;

      [[nodiscard]] auto get_surface(vk::Instance instance) const -> result<vk::UniqueSurfaceKHR>;

   private:
      std::string title{"Vermillon standard window"};
      std::uint32_t width{1080U};
      std::uint32_t height{720U};

      wnd_ptr p_wnd;
   };
} // namespace ui
