#include <libowl/window.hpp>

#include <libowl/system.hpp>

namespace owl::inline v0
{
   void window::render(std::chrono::nanoseconds) {}

   [[nodiscard]] auto window::is_gui_thread() const noexcept -> bool
   {
      return m_system.is_gui_thread();
   }

   [[nodiscard]] auto window::title() const noexcept -> std::string_view { return m_title; }
   [[nodiscard]] auto window::surface() const noexcept -> const render_surface&
   {
      return m_surface;
   }

   void window::set_physical_device(ash::physical_device&& device) noexcept
   {
      m_physical_device = std::move(device);
      m_device =
         ash::device(ash::device_create_info{.physical = m_physical_device, .logger = m_logger});
   }

   window::window(system& system, std::string_view title, spdlog::logger& logger) :
      m_system(system), m_logger(logger), m_title(title)
   {}

   [[nodiscard]] auto window::logger() const noexcept -> spdlog::logger& { return m_logger; }

   void window::set_surface(render_surface&& surface) { m_surface = std::move(surface); }
} // namespace owl::inline v0
