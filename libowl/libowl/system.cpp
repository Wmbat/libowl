#include <chrono>
#include <libowl/system.hpp>

#include <libowl/chrono.hpp>
#include <libowl/version.hpp>
#include <libowl/window.hpp>
#include <libowl/window/x11_support.hpp>
#include <libowl/window/x11_window.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <libreglisse/maybe.hpp>
#include <libreglisse/operations/and_then.hpp>
#include <libreglisse/result.hpp>

#include <fmt/chrono.h>

#include <xcb/xcb.h>

using reglisse::maybe;
using reglisse::none;

namespace owl::inline v0
{
   namespace
   {

   } // namespace

   template <typename Type, typename Err>
   auto throw_if_err(reglisse::result<Type, Err>&& res) -> Type
   {
      if (res.is_ok())
      {
         return std::move(res).take();

         // throw
      }
   }

   system::system(std::string_view app_name, mannele::log_ptr logger) :
      m_logger(logger), m_instance({.app_info = {.name = app_name, .version = {}},
                                    .eng_info = {.name = "owl", .version = library_version},
                                    .enabled_extension_names = {"VK_KHR_surface"},
                                    .logger = m_logger}),
      m_x11_connection(x11::connect_to_server(m_logger).take()),
      m_monitors(x11::list_available_monitors(m_x11_connection))
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

         poll_events();

         // handle events

         render(delta_time);
      } while (not exit_code);

      m_logger.info("shutting down");

      return exit_code.borrow();
   }

   void system::poll_events()
   {
      while (auto event = x11::poll_for_event(m_x11_connection))
      {
         switch (event->response_type & ~0x80) // NOLINT
         {
            case XCB_EXPOSE:
            {
               m_logger.debug("expose event");
               break;
            }
            case XCB_KEY_PRESS:
            {
               // NOLINTNEXTLINE
               auto* key_press_event = reinterpret_cast<xcb_key_press_event_t*>(event.get());

               m_logger.debug("key \"{}\" pressed at {}", key_press_event->detail,
                              key_press_event->time);
               break;
            }
            case XCB_KEY_RELEASE:
            {
               m_logger.debug("key release event");
               break;
            }
            case XCB_BUTTON_PRESS:
            {
               m_logger.debug("button press event");
               break;
            }
            case XCB_BUTTON_RELEASE:
            {
               m_logger.debug("button release event");
               break;
            }
            case XCB_ENTER_NOTIFY:
            {
               m_logger.debug("enter window event");
               break;
            }
            case XCB_LEAVE_NOTIFY:
            {
               m_logger.debug("leave window event");
               break;
            }
            default:
            {
               m_logger.warning("unknown event: {}", event->response_type);

               break;
            }
         }
      }
   }
   void system::render(std::chrono::nanoseconds) {}

   auto system::make_window(std::string_view name) -> window&
   {
      std::unique_ptr p_window =
         std::make_unique<x11::window>(x11::window_create_info{.name = name,
                                                               .connection = m_x11_connection,
                                                               .instance = m_instance,
                                                               .p_target_monitor = &m_monitors[0],
                                                               .logger = m_logger});

      return add_window(std::move(p_window));
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
