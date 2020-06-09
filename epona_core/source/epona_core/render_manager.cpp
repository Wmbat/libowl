/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include "epona_core/render_manager.hpp"
#include "epona_core/details/logger.hpp"
#include "epona_core/details/monads/either.hpp"
#include "epona_core/details/monads/option.hpp"
#include "epona_core/vk/device.hpp"
#include "epona_core/vk/instance.hpp"
#include "epona_core/vk/physical_device.hpp"

#include <string>

namespace core
{
   render_manager::render_manager(core::window* p_wnd, core::logger* p_logger) :
      p_window{p_wnd}, p_logger{p_logger}, runtime{p_logger}
   {
      const auto handle_instance_error = [&](const vk::details::error& err) {
         LOG_ERROR_P(p_logger, "Failed to create instance: {1}", err.type.message());
         abort();
         return vk::instance{};
      };
      const auto handle_surface_error = [&](const vk::details::error& err) -> VkSurfaceKHR {
         LOG_ERROR_P(p_logger, "Failed to create surface {1}", err.type.message());
         abort();
         return VK_NULL_HANDLE;
      };
      const auto handle_gpu_error = [&](const vk::details::error& err) {
         LOG_ERROR_P(p_logger, "Failed to create instance: {1}", err.type.message());
         abort();
         return vk::physical_device{};
      };
      const auto handle_device_error = [&](const vk::details::error& err) {
         LOG_ERROR_P(p_logger, "Failed to create device: {1}", err.type.message());
         abort();
         return vk::device{};
      };

      // clang-format off
      instance = vk::instance_builder{}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name(engine_name)
         .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
         .build(runtime, p_logger)
         .right_map(handle_instance_error)
         .join();

      device = vk::device_builder{
         vk::physical_device_selector{instance, p_logger}
            .set_prefered_gpu_type(vk::physical_device::type::discrete)
            .set_surface(p_window->get_surface(instance.vk_instance)
               .right_map(handle_surface_error)
               .join())
            .allow_any_gpu_type()
            .require_present()
            .select()
            .right_map(handle_gpu_error)
            .join(), p_logger}
         .build()
         .right_map(handle_device_error)
         .join();
      // clang-format on
   }
} // namespace core
