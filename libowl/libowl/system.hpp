#ifndef LIBOWL_SYSTEM_HPP_
#define LIBOWL_SYSTEM_HPP_

#include <libowl/chrono.hpp>
#include <libowl/types.hpp>
#include <libowl/version.hpp>
#include <libowl/window/monitor.hpp>

#include <libowl/window/x11_support.hpp>

#include <libash/instance.hpp>
#include <libash/physical_device.hpp>

#include <libmannele/core/semantic_version.hpp>
#include <libmannele/logging/log_ptr.hpp>

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
      system(std::string_view app_name, mannele::log_ptr logger = nullptr);

      auto run() -> i32;

      auto make_window(std::string_view name) -> window&;

   private: 
      void poll_events();
      void render(std::chrono::nanoseconds delta_time);

      auto add_window(std::unique_ptr<window>&& wnd) -> window&;

   private:
      mannele::log_ptr m_logger;

      ash::instance m_instance;

      x11::unique_connection m_x11_connection;

      std::vector<monitor> m_monitors;
      std::vector<std::unique_ptr<window>> m_windows;
   };
} // namespace owl::inline v0

#endif // LIBOWL_SYSTEM_HPP_
