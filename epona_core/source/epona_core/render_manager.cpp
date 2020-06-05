/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include "epona_core/render_manager.hpp"
#include "epona_core/details/logger.hpp"
#include "epona_core/vk/device.hpp"
#include "epona_core/vk/instance.hpp"
#include "epona_core/vk/physical_device.hpp"

namespace core
{
   render_manager::render_manager(core::window* p_wnd, core::logger* p_logger) :
      p_window{p_wnd}, p_logger{p_logger}, runtime{p_logger}
   {
      // clang-format off
      auto instance_res = vk::instance_builder{}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name(engine_name)
         .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
         .build(runtime, p_logger);
      // clang-format on

      if (!instance_res)
      {
         LOG_ERROR_P(
            p_logger, "Failed to create instance: {1}", instance_res.error_type().message());
      }

      instance = std::move(instance_res.value());

      auto surface_res = p_window->get_surface(instance.vk_instance);
      if (!surface_res)
      {
         LOG_ERROR(p_logger, "Failed to create surface");
      }

      // clang-format off
      auto gpu_res = vk::physical_device_selector{instance, p_logger}
         .set_prefered_gpu_type(vk::physical_device::type::discrete)
         .set_surface(surface_res.value())
         .allow_any_gpu_type()
         .require_present()
         .select();
      // clang-format on

      if (!gpu_res)
      {
         LOG_ERROR_P(p_logger, "Failed to create instance: {1}", gpu_res.error_type().message());
      }

      // clang-format off
      auto device_res = vk::device_builder{std::move(gpu_res.value()), p_logger}
         .build();
      // clang-format on

      if (!device_res)
      {
         LOG_ERROR_P(p_logger, "Failed to create device: {1}", gpu_res.error_type().message());
      }

      device = std::move(device_res.value());
   }
} // namespace core
