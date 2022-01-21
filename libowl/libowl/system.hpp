#ifndef LIBOWL_SYSTEM_HPP_
#define LIBOWL_SYSTEM_HPP_

#include <libowl/chrono.hpp>
#include <libowl/gui/event/command.hpp>
#include <libowl/gui/event/focus_event.hpp>
#include <libowl/gui/event/structure_changed_event.hpp>
#include <libowl/gui/monitor.hpp>
#include <libowl/types.hpp>
#include <libowl/version.hpp>
#include <libowl/window.hpp>

#include <libash/instance.hpp>
#include <libash/physical_device.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <spdlog/spdlog.h>

#include <thread>

namespace owl::inline v0
{
   static constexpr auto library_version = mannele::semantic_version{
      .major = LIBOWL_VERSION_MAJOR, .minor = LIBOWL_VERSION_MINOR, .patch = LIBOWL_VERSION_PATCH};

   /**
    * @brief Initializes
    */
   class system
   {
   public:
      system(std::string_view app_name);

      auto run() -> i32;

      auto make_window(std::string_view name) -> window&;

      [[nodiscard]] auto is_gui_thread() const noexcept -> bool;

   private:
      void handle_events();
      void handle_structure_changed_event(const structure_changed_event& event);
      void handle_focus_event(const focus_event& event);
      void handle_command(command cmd);

      void render(std::chrono::nanoseconds delta_time);

      auto add_window(unique_window&& wnd) -> window&;

   private:
      spdlog::logger m_logger;

      ash::instance m_instance;

      x11::connection m_xserver_connection;

      std::vector<monitor> m_monitors;
      std::vector<unique_window> m_windows;
      window* m_window_in_focus = nullptr;

      std::thread::id m_thread_id;
   };
} // namespace owl::inline v0

#endif // LIBOWL_SYSTEM_HPP_
