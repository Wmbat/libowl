#include <libowl/system.hpp>

#include <libowl/version.hpp>
#include <libowl/window.hpp>
#include <libowl/window/x11_support.hpp>
#include <libowl/window/x11_window.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <libreglisse/operations/and_then.hpp>
#include <libreglisse/result.hpp>

#include <xcb/xcb.h>

namespace owl::inline v0
{
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

   auto system::run() -> i32 { return EXIT_SUCCESS; }

   auto system::make_window(std::string_view name) -> window*
   {
      std::unique_ptr p_window =
         std::make_unique<x11::window>(x11::window_create_info{.name = name,
                                                               .connection = m_x11_connection,
                                                               .instance = m_instance,
                                                               .p_target_monitor = &m_monitors[0],
                                                               .logger = m_logger});

      auto phys_device_res = ash::find_most_suitable_gpu({.instance = m_instance,
                                                          .surface = p_window->surface(),
                                                          .require_transfer_queue = true,
                                                          .require_compute_queue = true,
                                                          .minimum_version = m_instance.version()});
      if (phys_device_res.is_err())
      {
         // return an error
      }

      m_logger.info(R"(rendering window "{}" using physical device "{}")", p_window->title(),
                    phys_device_res.borrow().properties.deviceName);

      p_window->set_physical_device(std::move(phys_device_res).take());

      m_windows.push_back(std::move(p_window));

      return m_windows.back().get();
   }
} // namespace owl::inline v0
