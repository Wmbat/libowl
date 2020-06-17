/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include "epona_core/render_manager.hpp"
#include "epona_core/details/logger.hpp"
#include "epona_core/details/monad/either.hpp"
#include "epona_core/details/monad/maybe.hpp"
/*
#include "epona_core/vk/device.hpp"
#include "epona_core/vk/instance.hpp"
#include "epona_core/vk/physical_device.hpp"
*/

#include <functional>
#include <string>

namespace core
{
   /*
   vk::instance handle_instance_error(const vk::detail::error& err, logger* p_logger);
   vk::physical_device handle_physical_device_error(const vk::detail::error& err, logger* p_logger);
   vk::device handle_device_error(const vk::detail::error& err, logger* p_logger);

   */
   render_manager::render_manager(window* const p_wnd, logger* const p_logger) :
      p_window{p_wnd}, p_logger{p_logger}
   {
   }

   /*
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
   */
} // namespace core
