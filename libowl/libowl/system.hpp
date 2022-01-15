#ifndef LIBOWL_SYSTEM_HPP_
#define LIBOWL_SYSTEM_HPP_

#include <libowl/chrono.hpp>
#include <libowl/gui/monitor.hpp>
#include <libowl/types.hpp>
#include <libowl/version.hpp>

#include <libash/instance.hpp>
#include <libash/physical_device.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <spdlog/spdlog.h>

#include <thread>

namespace owl::inline v0
{
   static constexpr auto library_version = mannele::semantic_version{
      .major = LIBOWL_VERSION_MAJOR, .minor = LIBOWL_VERSION_MINOR, .patch = LIBOWL_VERSION_PATCH};

   class window;

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
      void render(std::chrono::nanoseconds delta_time);

      auto add_window(std::unique_ptr<window>&& wnd) -> window&;

   private:
      spdlog::logger m_logger;

      ash::instance m_instance;

      x11::connection m_xserver_connection;

      std::vector<monitor> m_monitors;
      std::vector<std::unique_ptr<window>> m_windows;

      std::thread::id m_thread_id;
   };
} // namespace owl::inline v0

#endif // LIBOWL_SYSTEM_HPP_
