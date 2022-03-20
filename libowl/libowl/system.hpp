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
    * @brief Central starting point of the library. Used for keeping track of all the windows and
    * events for the GUI
    *
    * This class will initialize all the necessary components such as the underlying supported
    * window architecture (x11) as well as initializing the necessary vulkan components for the GPU
    * rendering of the GUI.
    */
   class system
   {
   public:
      /**
       * @brief Initializes the gui system
       *
       * @param[in] app_name The name of the app
       */
      explicit system(std::string_view app_name);

      /**
       * @brief The main loop
       */
      auto run() -> i32;

      /**
       * @brief Create a new window
       *
       * @param[in] name The name of the window
       *
       * @return A reference to the newly created window
       */
      auto make_window(std::string_view name) -> window&;

      [[nodiscard]] auto is_gui_thread() const noexcept -> bool;

   private:
      void handle_events();
      void handle_structure_changed_event(structure_changed_event const& event);
      void handle_focus_event(focus_event const& event);
      void handle_command(command cmd);

      void render(std::chrono::nanoseconds delta_time);

      /**
       * @brief Add a window to the list of root windows stored by the system
       *
       * @param[in] wnd The window to add_window
       *
       * @return A reference to the just added window.
       */
      auto add_window(unique_window&& wnd) -> window&;

   private:
      spdlog::logger m_logger;

      ash::instance m_instance;
      std::vector<ash::physical_device> m_physical_devices;

      x11::connection m_xserver_connection;

      std::vector<monitor> m_monitors;
      std::vector<unique_window> m_windows;
      window* m_window_in_focus = nullptr;

      std::thread::id m_thread_id;
   };
} // namespace owl::inline v0

#endif // LIBOWL_SYSTEM_HPP_
