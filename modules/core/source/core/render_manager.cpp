/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include <core/render_manager.hpp>

#include <vkn/device.hpp>
#include <vkn/physical_device.hpp>

#include <util/logger.hpp>
#include <util/monad/either.hpp>
#include <util/monad/maybe.hpp>

namespace core
{
   auto handle_instance_error(const vkn::error&, util::logger* const) -> vkn::instance;
   auto handle_physical_device_error(const vkn::error&, util::logger* const)
      -> vkn::physical_device;
   auto handle_device_error(const vkn::error&, util::logger* const) -> vkn::device;
   auto handle_surface_error(const vkn::error&, util::logger* const) -> vk::SurfaceKHR;
   auto handle_swapchain_error(const vkn::error&, util::logger* const) -> vkn::swapchain;

   render_manager::render_manager(gfx::window* const p_wnd, util::logger* const plogger) :
      m_pwindow{p_wnd}, m_plogger{plogger}, m_loader{m_plogger}
   {
      // clang-format off
      m_instance = vkn::instance::builder{m_loader, m_plogger}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name(m_engine_name)
         .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
         .build()
         .error_map([plogger](auto&& err) { return handle_instance_error(err, plogger); })
         .join();

      m_device = vkn::device::builder{m_loader,
         vkn::physical_device::selector{m_instance, m_plogger}
            .set_surface(p_wnd->get_surface(m_instance.value())
               .error_map([plogger](auto&& err) { return handle_surface_error(err, plogger); })
               .join())
            .set_prefered_gpu_type(vkn::physical_device::type::discrete)
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

      m_swapchain = vkn::swapchain::builder{m_device}
         .set_desired_format({vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear})
         .set_desired_present_mode(vk::PresentModeKHR::eMailbox)
         .add_fallback_present_mode(vk::PresentModeKHR::eFifo)
         .set_clipped(true)
         .set_composite_alpha_flags(vk::CompositeAlphaFlagBitsKHR::eOpaque)
         .build()
         .error_map([plogger](auto&& err){ return handle_swapchain_error(err, plogger); })
         .join();
      // clang-format on
   }

   auto handle_instance_error(const vkn::error& err, util::logger* const m_plogger) -> vkn::instance
   {
      log_error(m_plogger, "Failed to create instance: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
   auto handle_physical_device_error(const vkn::error& err, util::logger* const m_plogger)
      -> vkn::physical_device
   {
      log_error(
         m_plogger, "Failed to create physical device: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
   auto handle_device_error(const vkn::error& err, util::logger* const m_plogger) -> vkn::device
   {
      log_error(m_plogger, "Failed to create device: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }

   auto handle_surface_error(const vkn::error& err, util::logger* const m_plogger) -> vk::SurfaceKHR
   {
      log_error(m_plogger, "Failed to create surface: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
   auto handle_swapchain_error(const vkn::error& err, util::logger* const plogger) -> vkn::swapchain
   {
      log_error(plogger, "Failed to create surface: {0}", std::make_tuple(err.type.message()));
      abort();

      return {};
   }
} // namespace core
