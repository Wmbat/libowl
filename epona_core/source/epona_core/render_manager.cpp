/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include "epona_core/render_manager.hpp"
#include "epona_core/details/logger.hpp"
#include "epona_core/details/monad/either.hpp"
#include "epona_core/details/monad/option.hpp"
#include "epona_core/vk/device.hpp"
#include "epona_core/vk/instance.hpp"
#include "epona_core/vk/physical_device.hpp"

#include <functional>
#include <string>

namespace core
{
   vk::instance handle_instance_error(const vk::detail::error& err, logger* p_logger);
   vk::physical_device handle_physical_device_error(const vk::detail::error& err, logger* p_logger);
   vk::device handle_device_error(const vk::detail::error& err, logger* p_logger);

   render_manager::render_manager(window* const p_wnd, logger* const p_logger) :
      p_window{p_wnd}, p_logger{p_logger}, runtime{p_logger}
   {
      using namespace std::placeholders;

      // clang-format off
      instance = vk::instance_builder{runtime, p_logger}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name(engine_name)
         .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
         .build()
         .right_map(std::bind(handle_instance_error, _1, p_logger))
         .join();

      device = vk::device_builder{
         vk::physical_device_selector{instance, p_logger}
            .set_prefered_gpu_type(vk::physical_device::type::discrete)
            .set_surface(p_window->get_surface(instance.vk_instance)
               .right_map([&](const vk::detail::error& err) -> VkSurfaceKHR {
                  LOG_ERROR_P(p_logger, "Failed to create surface {1}", err.type.message());
                  abort();
                  return VK_NULL_HANDLE;
               })
               .join())
            .allow_any_gpu_type()
            .require_present()
            .select()
            .right_map(std::bind(handle_physical_device_error, _1, p_logger))
            .join(), p_logger}
         .build()
         .right_map(std::bind(handle_device_error, _1, p_logger))
         .join();
      // clang-format on
   }

   vk::instance handle_instance_error(const vk::detail::error& err, logger* p_logger)
   {
      LOG_ERROR_P(p_logger, "Failed to create instance: {1}", err.type.message());
      abort();
      return vk::instance{};
   }
   vk::physical_device handle_physical_device_error(const vk::detail::error& err, logger* p_logger)
   {
      LOG_ERROR_P(p_logger, "Failed to create physical device: {1}", err.type.message());
      abort();
      return vk::physical_device{};
   }
   vk::device handle_device_error(const vk::detail::error& err, logger* p_logger)
   {
      LOG_ERROR_P(p_logger, "Failed to create device: {1}", err.type.message());
      abort();
      return vk::device{};
   }
} // namespace core
