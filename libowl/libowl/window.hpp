#ifndef LIBOWL_WINDOW_HPP_
#define LIBOWL_WINDOW_HPP_

#include <libowl/gfx/render_surface.hpp>
#include <libowl/system.hpp>
#include <libowl/window/monitor.hpp>

#include <libash/device.hpp>

// C++ Standard Library

#include <functional>
#include <memory>

namespace owl::inline v0
{
   class window
   {
   public:
      [[nodiscard]] auto title() const noexcept -> std::string_view;
      [[nodiscard]] auto surface() const noexcept -> const render_surface&;

      void set_physical_device(ash::physical_device&& device) noexcept;

   protected:
      window(std::string_view title, mannele::log_ptr logger);

      [[nodiscard]] auto logger() const noexcept -> mannele::log_ptr;

      void set_surface(render_surface&& surface);

   private:
      mannele::log_ptr m_logger;

      std::string m_title;

      render_surface m_surface;
      ash::physical_device m_physical_device;
      ash::device m_device;
   };
} // namespace owl::inline v0

#endif // LIBOWL_WINDOW_HPP_
