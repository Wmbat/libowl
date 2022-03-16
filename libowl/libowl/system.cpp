#include <libowl/system.hpp>

#include <libowl/chrono.hpp>
#include <libowl/detail/visit_helper.hpp>
#include <libowl/gfx/device.hpp>
#include <libowl/detail/x11/window.hpp>
#include <libowl/gui/event/event.hpp>
#include <libowl/gui/monitor.hpp>
#include <libowl/version.hpp>
#include <libowl/window.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <fmt/chrono.h>

#include <range/v3/algorithm/remove.hpp>

#include <chrono>

#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace owl::inline v0
{
   namespace
   {
      auto create_logger(std::string_view name) -> spdlog::logger
      {
         auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
         console_sink->set_level(spdlog::level::trace);
         console_sink->set_pattern("[%n] [%^%l%$] %v");

         auto file_sink =
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{name} + ".logs", true);
         file_sink->set_pattern("[%H:%M:%S.%f] [%n] [%^%l%$] %v");
         file_sink->set_level(spdlog::level::trace);

         auto logger = spdlog::logger(std::string(name), {console_sink, file_sink});
         logger.set_level(spdlog::level::trace);
         return logger;
      }
   } // namespace

   system::system(std::string_view app_name) :
      m_logger(create_logger(app_name)),
      m_instance({.app_info = {.name = app_name, .version = {}},
                  .eng_info = {.name = "owl", .version = library_version},
                  .enabled_extension_names = {"VK_KHR_surface"},
                  .enabled_layer_names = {},
                  .logger = m_logger}),
      m_physical_devices(ash::enumerate_physical_devices(m_instance)),
      m_xserver_connection(x11::connect_to_server(m_logger).value()),
      m_monitors(list_available_monitors(m_xserver_connection)),
      m_thread_id(std::this_thread::get_id())
   {}

   auto system::run() -> i32
   {
      std::optional<int> exit_code = std::nullopt;

      // Switch to UTC
      auto curr_time = sys_nanosecond(std::chrono::system_clock::now());

      do
      {
         auto const new_time = sys_nanosecond(std::chrono::system_clock::now());
         auto const delta_time = new_time - curr_time;
         curr_time = new_time;

         handle_events();

         render(delta_time);

         if (std::empty(m_windows))
         {
            m_logger.info("No windows open");

            exit_code = 0;
         }
      } while (not exit_code);

      m_logger.info("shutting down");

      return exit_code.value();
   }

   void system::handle_events()
   {
      using detail::overloaded;

      while (auto const event = poll_for_event(m_xserver_connection))
      {
         // clang-format off
         std::visit(
            overloaded{
               [&](key_event const& e) { m_window_in_focus->handle_event(e); }, 
               [](mouse_button_event const&) {},
               [](mouse_movement_event const&) {},
               [&](structure_changed_event const& e) { handle_structure_changed_event(e); },
               [&](focus_event const& e) { handle_focus_event(e); },
               [&](command cmd) { handle_command(cmd); } },
            event.value());
         // clang-format on
      }
   }

   void system::handle_structure_changed_event(structure_changed_event const& event)
   {
      auto const it = std::ranges::find(m_windows, event.window_id, &window::id);
      if (it != std::end(m_windows))
      {
         m_logger.debug("window \"{}\" has been moved to {}", (*it)->title(), event.dimension);
      }
      else
      {
         m_logger.warn("window with id {} was not found!", event.window_id);
      }
   }
   void system::handle_focus_event(focus_event const& event)
   {
      if (event.type == focus_type::in)
      {
         auto const it = std::ranges::find(m_windows, event.window_id, &window::id);

         if (it != std::end(m_windows))
         {
            m_window_in_focus = it->get();

            m_logger.info("window \"{}\" is now in focus", m_window_in_focus->title());
         }
         else
         {
            m_logger.warn("window with id {} was not found!", event.window_id);
         }
      }
   }
   void system::handle_command(command cmd)
   {
      if (cmd == command::close_window)
      {
         m_logger.debug("closing window event");

         if (m_window_in_focus)
         {
            const auto it = std::ranges::find(m_windows, m_window_in_focus, &unique_window::get);
            if (it != std::end(m_windows))
            {
               m_logger.info("window \"{}\" closed", m_window_in_focus->title());

               m_windows.erase(ranges::remove(m_windows, m_window_in_focus, &unique_window::get),
                               std::end(m_windows));
               m_window_in_focus = nullptr;
            }
         }
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
      return add_window(
         std::make_unique<x11::window>(x11::window_create_info{.p_system = this,
                                                               .name = name,
                                                               .conn = m_xserver_connection,
                                                               .instance = m_instance,
                                                               .target_monitor = m_monitors[0],
                                                               .logger = m_logger}));
   }

   [[nodiscard]] auto system::is_gui_thread() const noexcept -> bool
   {
      return m_thread_id == std::this_thread::get_id();
   }

   auto system::add_window(std::unique_ptr<window>&& wnd) -> window&
   {
      auto results = ash::find_most_suitable_physical_device(
         m_physical_devices, {.surface = wnd->target().surface(),
                              .require_transfer_queue = true,
                              .require_compute_queue = true,
                              .minimum_version = m_instance.version(),
                              .required_extensions = std::vector({VK_KHR_SWAPCHAIN_EXTENSION_NAME}),
                              .desired_extensions = {}});

      if (!results)
      {
         m_logger.error("failed to find suitable GPU");

         throw std::move(results).error();
      }

      auto const& result_data = results.value();

      m_logger.info(R"(rendering window "{}" using physical device "{}")", wnd->title(),
                    result_data.p_physical_device->properties.deviceName);

      wnd->set_device(gfx::device(result_data.p_physical_device, result_data.queues_to_create,
                                  result_data.extension_to_enable, m_logger));

      m_windows.push_back(std::move(wnd));

      return *m_windows.back().get();
   }
} // namespace owl::inline v0
