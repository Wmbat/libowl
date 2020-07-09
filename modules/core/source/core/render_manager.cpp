/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include <core/graphics/vkn/device.hpp>
#include <core/graphics/vkn/physical_device.hpp>
#include <core/render_manager.hpp>

#include <util/logger.hpp>
#include <util/monad/either.hpp>
#include <util/monad/maybe.hpp>

namespace core
{
   auto handle_instance_error(const gfx::vkn::error&, util::logger* const) -> gfx::vkn::instance;
   auto handle_physical_device_error(const gfx::vkn::error&, util::logger* const)
      -> gfx::vkn::physical_device;
   auto handle_device_error(const gfx::vkn::error&, util::logger* const) -> gfx::vkn::device;
   auto handle_surface_error(const gfx::vkn::error&, util::logger* const) -> vk::SurfaceKHR;

   render_manager::render_manager(gfx::window* const p_wnd, util::logger* const plogger) :
      m_pwindow{p_wnd}, m_plogger{plogger}, m_loader{m_plogger}
   {
      // clang-format off
      m_instance = gfx::vkn::instance::builder{m_loader, m_plogger}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name(m_engine_name)
         .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
         .build()
         .error_map([plogger](auto&& err) { return handle_instance_error(err, plogger); })
         .join();

      m_device = gfx::vkn::device::builder{m_loader,
         gfx::vkn::physical_device::selector{m_instance, m_plogger}
            .set_surface(p_wnd->get_surface(m_instance.value())
               .error_map([plogger](auto&& err) { return handle_surface_error(err, plogger); })
               .join())
            .set_prefered_gpu_type(gfx::vkn::physical_device::type::discrete)
            .allow_any_gpu_type()
            .require_present()
            .select()
            .error_map([plogger](auto&& err) { 
               return handle_physical_device_error(err, plogger); 
            })
            .join(), m_plogger}
         .build()
         .error_map([plogger](auto&& PH1) { return handle_device_error(PH1, plogger); })
         .join();

      auto test = gfx::vkn::swapchain::builder{m_device};
      // clang-format on
   }

   auto handle_instance_error(const gfx::vkn::error& err, util::logger* const m_plogger)
      -> gfx::vkn::instance
   {
      log_error(m_plogger, "Failed to create instance: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
   auto handle_physical_device_error(const gfx::vkn::error& err, util::logger* const m_plogger)
      -> gfx::vkn::physical_device
   {
      log_error(
         m_plogger, "Failed to create physical device: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
   auto handle_device_error(const gfx::vkn::error& err, util::logger* const m_plogger)
      -> gfx::vkn::device
   {
      log_error(m_plogger, "Failed to create device: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }

   auto handle_surface_error(const gfx::vkn::error& err, util::logger* const m_plogger)
      -> vk::SurfaceKHR
   {
      log_error(m_plogger, "Failed to create surface: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
} // namespace core
