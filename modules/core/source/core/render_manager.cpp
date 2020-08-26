#include <core/render_manager.hpp>

#include <vkn/device.hpp>
#include <vkn/physical_device.hpp>
#include <vkn/shader.hpp>

#include <util/containers/dynamic_array.hpp>
#include <util/logger.hpp>

#include <monads/maybe.hpp>

#include <limits>

namespace core
{
   auto handle_physical_device_error(const vkn::error&, util::logger* const)
      -> vkn::physical_device;
   auto handle_device_error(const vkn::error&, util::logger* const) -> vkn::device;
   auto handle_surface_error(const vkn::error&, util::logger* const) -> vk::SurfaceKHR;
   auto handle_swapchain_error(const vkn::error&, util::logger* const) -> vkn::swapchain;

   render_manager::render_manager(gfx::window* const p_wnd, util::logger* const plogger) :
      mp_window{p_wnd}, mp_logger{plogger}, m_loader{mp_logger}
   {
      m_instance =
         vkn::instance::builder{m_loader, mp_logger}
            .set_application_name("")
            .set_application_version(0, 0, 0)
            .set_engine_name(m_engine_name)
            .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
            .build()
            .map_error([plogger](auto&& err) {
               log_error(plogger, "[core] Failed to create instance: {0}", err.type.message());
               abort();

               return vkn::instance{};
            })
            .join();

      m_device =
         vkn::device::builder{m_loader,
                              vkn::physical_device::selector{m_instance, mp_logger}
                                 .set_surface(mp_window->get_surface(m_instance.value())
                                                 .map_error([plogger](auto&& err) {
                                                    return handle_surface_error(err, plogger);
                                                 })
                                                 .join())
                                 .set_preferred_gpu_type(vkn::physical_device::type::discrete)
                                 .allow_any_gpu_type()
                                 .require_present()
                                 .select()
                                 .map_error([plogger](auto&& err) {
                                    return handle_physical_device_error(err, plogger);
                                 })
                                 .join(),
                              m_instance.version(), mp_logger}
            .build()
            .map_error([plogger](auto&& err) {
               return handle_device_error(err, plogger);
            })
            .join();

      m_swapchain =
         vkn::swapchain::builder{m_device, plogger}
            .set_desired_format({vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear})
            .set_desired_present_mode(vk::PresentModeKHR::eMailbox)
            .add_fallback_present_mode(vk::PresentModeKHR::eFifo)
            .set_clipped(true)
            .set_composite_alpha_flags(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .build()
            .map_error([plogger](auto&& err) {
               return handle_swapchain_error(err, plogger);
            })
            .join();

      m_render_pass = vkn::render_pass::builder{m_device, m_swapchain, plogger}
                         .build()
                         .map_error([plogger](auto&& err) {
                            log_error(plogger, "[core] Failed to create render pass: \"{0}\"",
                                      err.type.message());
                            abort();

                            return vkn::render_pass{};
                         })
                         .join();

      m_framebuffers.reserve(std::size(m_swapchain.image_views()));
      for (const auto& img_view : m_swapchain.image_views())
      {
         m_framebuffers.emplace_back(vkn::framebuffer::builder{m_device, m_render_pass, plogger}
                                        .add_attachment(img_view.get())
                                        .set_buffer_width(m_swapchain.extent().width)
                                        .set_buffer_height(m_swapchain.extent().height)
                                        .set_layer_count(1u)
                                        .build()
                                        .map_error([plogger](vkn::error&& err) {
                                           log_error(plogger,
                                                     "[core] Failed to create framebuffer: \"{0}\"",
                                                     err.type.message());
                                           abort();

                                           return vkn::framebuffer{};
                                        })
                                        .join());
      }

      m_shader_codex =
         shader_codex::builder{m_device, plogger}
            .add_shader_filepath("resources/shaders/test_shader.vert")
            .add_shader_filepath("resources/shaders/test_shader.frag")
            .allow_caching(false)
            .build()
            .map_error([plogger](auto&& err) {
               log_error(plogger, "[core] Failed to create shader codex: \"{0}\"", err.message());
               abort();

               return shader_codex{};
            })
            .join();

      m_command_pool =
         vkn::command_pool::builder{m_device, plogger}
            .set_queue_family_index(
               m_device.get_queue_index(vkn::queue::type::graphics)
                  .map_error([&](auto&& err) {
                     log_error(plogger, "[core] No usable graphics queues found: \"{0}\"",
                               err.type.message());
                     abort();

                     return 0u;
                  })
                  .join())
            .set_primary_buffer_count(std::size(m_framebuffers))
            .build()
            .map_error([plogger](auto&& err) {
               log_error(plogger, "[core] Failed to create command pool: \"{0}\"",
                         err.type.message());

               abort();

               return vkn::command_pool{};
            })
            .join();

      m_graphics_pipeline =
         vkn::graphics_pipeline::builder{m_device, m_render_pass, plogger}
            .add_shader(m_shader_codex.get_shader("test_shader.vert"))
            .add_shader(m_shader_codex.get_shader("test_shader.frag"))
            .add_viewport({.x = 0.0f,
                           .y = 0.0f,
                           .width = static_cast<float>(m_swapchain.extent().width),
                           .height = static_cast<float>(m_swapchain.extent().height),
                           .minDepth = 0.0f,
                           .maxDepth = 1.0f},
                          {.offset = {0, 0}, .extent = m_swapchain.extent()})
            .set_topology(vk::PrimitiveTopology::eTriangleList)
            .enable_primitive_restart(false)
            .build()
            .map_error([&](vkn::error&& err) {
               log_error(plogger, "[core] Failed to create graphics pipeline: \"{0}\"",
                         err.type.message());

               abort();

               return vkn::graphics_pipeline{};
            })
            .join();

      for (auto& semaphore : m_image_available_semaphores)
      {
         semaphore = vkn::semaphore::builder{m_device, mp_logger}
                        .build()
                        .map_error([&](vkn::error&& err) {
                           log_error(plogger, "[core] Failed to create semaphore: \"{0}\"",
                                     err.type.message());
                           abort();

                           return vkn::semaphore{};
                        })
                        .join();
      }

      for (auto& semaphore : m_render_finished_semaphores)
      {
         semaphore = vkn::semaphore::builder{m_device, mp_logger}
                        .build()
                        .map_error([&](vkn::error&& err) {
                           log_error(plogger, "[core] Failed to create semaphore: \"{0}\"",
                                     err.type.message());
                           abort();

                           return vkn::semaphore{};
                        })
                        .join();
      }

      for (auto& fence : m_in_flight_fences)
      {
         fence = vkn::fence::builder{m_device, mp_logger}
                    .set_signaled()
                    .build()
                    .map_error([&](vkn::error&& err) {
                       log_error(plogger, "[core] Failed to create in flight fence: \"{0}\"",
                                 err.type.message());
                       abort();

                       return vkn::fence{};
                    })
                    .join();
      }

      m_images_in_flight.resize(std::size(m_swapchain.image_views()), {nullptr});

      util::log_info(mp_logger, "[core] recording main rendering command buffers");
      for (std::size_t i = 0; const auto& buffer : m_command_pool.primary_cmd_buffers())
      {
         buffer.begin({.pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr});

         const auto clear_colour = vk::ClearValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}};
         buffer.beginRenderPass({.pNext = nullptr,
                                 .renderPass = m_render_pass.value(),
                                 .framebuffer = m_framebuffers[i++].value(),
                                 .renderArea = {{0, 0}, m_swapchain.extent()},
                                 .clearValueCount = 1,
                                 .pClearValues = &clear_colour},
                                vk::SubpassContents::eInline);

         buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline.value());

         buffer.draw(3u, 1u, 0u, 0u);

         buffer.endRenderPass();
         buffer.end();
      }
   }

   void render_manager::render_frame()
   {
      m_device->waitForFences({vkn::value(m_in_flight_fences.at(m_current_frame))}, true,
                              std::numeric_limits<std::uint64_t>::max());

      auto [image_res, image_index] = m_device->acquireNextImageKHR(
         vkn::value(m_swapchain), std::numeric_limits<std::uint64_t>::max(),
         vkn::value(m_image_available_semaphores.at(m_current_frame)), nullptr);

      if (m_images_in_flight[image_index])
      {
         m_device->waitForFences({vkn::value(m_in_flight_fences.at(m_current_frame))}, true,
                                 std::numeric_limits<std::uint64_t>::max());
      }
      m_images_in_flight[image_index] = vkn::value(m_in_flight_fences.at(m_current_frame));

      const std::array wait_semaphores{
         vkn::value(m_image_available_semaphores.at(m_current_frame))};
      const std::array signal_semaphores{
         vkn::value(m_render_finished_semaphores.at(m_current_frame))};
      const std::array command_buffers{m_command_pool.primary_cmd_buffers()[image_index]};
      const std::array<vk::PipelineStageFlags, 1> wait_stages{
         vk::PipelineStageFlagBits::eColorAttachmentOutput};

      m_device->resetFences({vkn::value(m_in_flight_fences.at(m_current_frame))});

      const std::array submit_infos{
         vk::SubmitInfo{.waitSemaphoreCount = std::size(wait_semaphores),
                        .pWaitSemaphores = std::data(wait_semaphores),
                        .pWaitDstStageMask = std::data(wait_stages),
                        .commandBufferCount = std::size(command_buffers),
                        .pCommandBuffers = std::data(command_buffers),
                        .signalSemaphoreCount = std::size(signal_semaphores),
                        .pSignalSemaphores = std::data(signal_semaphores)}};

      try
      {
         const auto gfx_queue = *m_device.get_queue(vkn::queue::type::graphics).value();
         gfx_queue.submit(submit_infos, vkn::value(m_in_flight_fences.at(m_current_frame)));
      }
      catch (const vk::SystemError& err)
      {
         util::log_error(mp_logger, "[core] failed to submit graphics queue");
         abort();
      }

      const std::array swapchains{vkn::value(m_swapchain)};

      const auto present_queue = *m_device.get_queue(vkn::queue::type::present).value();
      if (present_queue.presentKHR({.waitSemaphoreCount = std::size(signal_semaphores),
                                    .pWaitSemaphores = std::data(signal_semaphores),
                                    .swapchainCount = std::size(swapchains),
                                    .pSwapchains = std::data(swapchains),
                                    .pImageIndices = &image_index}) != vk::Result::eSuccess)
      {
         util::log_error(mp_logger, "[core] failed to present present queue");
         abort();
      }

      m_current_frame = (m_current_frame + 1) % max_frames_in_flight;
   }

   void render_manager::wait() { m_device->waitIdle(); }

   auto handle_physical_device_error(const vkn::error& err, util::logger* const plogger)
      -> vkn::physical_device
   {
      log_error(plogger, "[core] Failed to create physical device: {0}", err.type.message());
      abort();

      return {};
   }
   auto handle_device_error(const vkn::error& err, util::logger* const plogger) -> vkn::device
   {
      log_error(plogger, "[core] Failed to create device: {0}", err.type.message());
      abort();

      return {};
   }

   auto handle_surface_error(const vkn::error& err, util::logger* const plogger) -> vk::SurfaceKHR
   {
      log_error(plogger, "[core] Failed to create surface: {0}", err.type.message());
      abort();

      return {};
   }
   auto handle_swapchain_error(const vkn::error& err, util::logger* const plogger) -> vkn::swapchain
   {
      log_error(plogger, "[core] Failed to create swapchain: {0}", err.type.message());
      abort();

      return {};
   }
} // namespace core
