/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include "epona_core/render_manager.hpp"
#include "epona_core/detail/logger.hpp"
#include "epona_core/detail/monad/either.hpp"
#include "epona_core/detail/monad/maybe.hpp"

#include <functional>
#include <string>

namespace core
{
   gfx::vkn::instance handle_instance_error(const gfx::vkn::error& err, logger* p_logger);
   /*
   vk::physical_device handle_physical_device_error(const vk::detail::error& err, logger* p_logger);
   vk::device handle_device_error(const vk::detail::error& err, logger* p_logger);
   */

   render_manager::render_manager(gfx::window* const p_wnd, logger* const p_logger) :
      p_window{p_wnd}, p_logger{p_logger}, loader{p_logger}
   {
      using namespace std::placeholders;

      instance = gfx::vkn::instance_builder{loader, p_logger}
                    .set_application_name("")
                    .set_application_version(0, 0, 0)
                    .set_engine_name(engine_name)
                    .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
                    .build()
                    .left_map(std::bind(handle_instance_error, _1, p_logger))
                    .join();
   }

   gfx::vkn::instance handle_instance_error(const gfx::vkn::error& err, logger* p_logger)
   {
      LOG_ERROR_P(p_logger, "Failed to create instance: {1}", err.type.message());
      abort();

      gfx::vkn::instance ret;
      return ret;
   }
   /*
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
   */
} // namespace core
