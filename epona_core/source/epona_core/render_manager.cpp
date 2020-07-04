/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include <epona_core/detail/logger.hpp>
#include <epona_core/detail/monad/either.hpp>
#include <epona_core/detail/monad/maybe.hpp>
#include <epona_core/graphics/vkn/device.hpp>
#include <epona_core/graphics/vkn/physical_device.hpp>
#include <epona_core/render_manager.hpp>

#include <functional>
#include <string>

namespace core
{
   gfx::vkn::instance handle_instance_error(const gfx::vkn::error&, logger* const);
   gfx::vkn::physical_device handle_physical_device_error(const gfx::vkn::error&, logger* const);
   gfx::vkn::device handle_device_error(const gfx::vkn::error&, logger* const);

   vk::SurfaceKHR handle_surface_error(const gfx::vkn::error&, logger* const);

   render_manager::render_manager(gfx::window* const p_wnd, logger* const p_logger) :
      p_window{p_wnd}, p_logger{p_logger}, loader{p_logger}
   {
      using namespace std::placeholders;

      // clang-format off
      instance = gfx::vkn::instance::builder{loader, p_logger}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name(engine_name)
         .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
         .build()
         .error_map(std::bind(handle_instance_error, _1, p_logger))
         .join();

      device = gfx::vkn::device::builder{loader,
         gfx::vkn::physical_device::selector{instance, p_logger}
            .set_surface(p_wnd->get_surface(instance.value())
               .error_map(std::bind(handle_surface_error, _1, p_logger))
               .join())
            .set_prefered_gpu_type(gfx::vkn::physical_device::type::discrete)
            .allow_any_gpu_type()
            .require_present()
            .select()
            .error_map(std::bind(handle_physical_device_error, _1, p_logger))
            .join(), p_logger}
         .build()
         .error_map(std::bind(handle_device_error, _1, p_logger))
         .join();

      auto test = gfx::vkn::swapchain::builder{device};
      // clang-format on
   }

   gfx::vkn::instance handle_instance_error(const gfx::vkn::error& err, logger* const p_logger)
   {
      log_error(p_logger, "Failed to create instance: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
   gfx::vkn::physical_device handle_physical_device_error(
      const gfx::vkn::error& err, logger* const p_logger)
   {
      log_error(
         p_logger, "Failed to create physical device: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
   gfx::vkn::device handle_device_error(const gfx::vkn::error& err, logger* const p_logger)
   {
      log_error(p_logger, "Failed to create device: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }

   vk::SurfaceKHR handle_surface_error(const gfx::vkn::error& err, logger* const p_logger)
   {
      log_error(p_logger, "Failed to create surface: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
} // namespace core
