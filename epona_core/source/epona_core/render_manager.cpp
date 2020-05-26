/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include "epona_core/render_manager.hpp"
#include "epona_core/details/logger.hpp"
#include "epona_core/vk/instance.hpp"

namespace core
{
   render_manager::render_manager(core::logger* p_logger) : p_logger{p_logger}, vk_runtime{p_logger}
   {
      // clang-format off
      auto instance_res = vk::instance_builder{}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name(engine_name)
         .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
         .enable_extension("VK_KHR_get_physical_device_properties2")
         .build(vk_runtime, p_logger);
      // clang-format on

      if (!instance_res)
      {
         LOG_ERROR_P(
            p_logger, "Failed to create instance: {1}", instance_res.error_type().message());
      }

      vk_instance = std::move(instance_res.value());
   }
} // namespace core
