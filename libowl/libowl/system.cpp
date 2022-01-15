#include <libowl/system.hpp>

#include <libowl/chrono.hpp>
#include <libowl/detail/x11/window.hpp>
#include <libowl/gui/event.hpp>
#include <libowl/gui/monitor.hpp>
#include <libowl/version.hpp>
#include <libowl/window.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <libreglisse/maybe.hpp>
#include <libreglisse/operations/and_then.hpp>
#include <libreglisse/result.hpp>

#include <fmt/chrono.h>

#include <chrono>

#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using reglisse::maybe;
using reglisse::none;

namespace owl::inline v0
{
   namespace
   {
      auto create_logger(std::string_view name) -> spdlog::logger
      {
         auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#if defined(LIBOWL_ENABLE_DEBUG_LOGGING)
         console_sink->set_level(spdlog::level::trace);
#else //
         console_sink->set_level(spdlog::level::info);
#endif
         console_sink->set_pattern("[%n] [%^%l%$] %v");

         auto file_sink =
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{name} + ".logs", true);
         file_sink->set_pattern("[%H:%M:%S.%f] [%n] [%^%l%$] %v");
         file_sink->set_level(spdlog::level::trace);

         return {std::string(name), {console_sink, file_sink}};
      }
   } // namespace

   system::system(std::string_view app_name) :
      m_logger(create_logger(app_name)),
      m_instance({.app_info = {.name = app_name, .version = {}},
                  .eng_info = {.name = "owl", .version = library_version},
                  .enabled_extension_names = {"VK_KHR_surface"},
                  .logger = m_logger}),
      m_xserver_connection(x11::connect_to_server(m_logger).take()),
      m_monitors(list_available_monitors(m_xserver_connection)),
      m_thread_id(std::this_thread::get_id())
   {}

   auto system::run() -> i32
   {
      maybe<int> exit_code = none;

      // Switch to UTC
      auto curr_time = sys_nanosecond(std::chrono::system_clock::now());

      do
      {
         const auto new_time = sys_nanosecond(std::chrono::system_clock::now());
         const auto delta_time = new_time - curr_time;
         curr_time = new_time;

         handle_events();

         render(delta_time);
      } while (not exit_code);

      m_logger.info("shutting down");

      return exit_code.borrow();
   }

   void system::handle_events()
   {
      while (auto poll_result = poll_for_event(m_xserver_connection))
      {
         const auto event = poll_result.borrow();
      }
   }
   void system::render(std::chrono::nanoseconds delta_time)
   {
      for (const auto& window : m_windows)
      {
         window->render(delta_time);
      }
   }

   auto system::make_window(std::string_view name) -> window&
   {
      std::unique_ptr p_window =
         std::make_unique<x11::window>(x11::window_create_info{.p_system = this,
                                                               .name = name,
                                                               .conn = m_xserver_connection,
                                                               .instance = m_instance,
                                                               .p_target_monitor = &m_monitors[0],
                                                               .logger = m_logger});

      return add_window(std::move(p_window));
   }

   [[nodiscard]] auto system::is_gui_thread() const noexcept -> bool
   {
      return m_thread_id == std::this_thread::get_id();
   }

   auto system::add_window(std::unique_ptr<window>&& wnd) -> window&
   {
      auto phys_device_res = ash::find_most_suitable_gpu(
         {.instance = m_instance,
          .surface = wnd->surface(),
          .require_transfer_queue = true,
          .require_compute_queue = true,
          .minimum_version = m_instance.version(),
          .required_extensions = std::vector({VK_KHR_SWAPCHAIN_EXTENSION_NAME})});

      if (phys_device_res.is_err())
      {
         m_logger.error("failed to find suitable GPU");
         // return an error
      }

      m_logger.info(R"(rendering window "{}" using physical device "{}")", wnd->title(),
                    phys_device_res.borrow().properties.deviceName);

      wnd->set_physical_device(std::move(phys_device_res).take());

      m_windows.push_back(std::move(wnd));

      return *m_windows.back().get();
   }
} // namespace owl::inline v0
