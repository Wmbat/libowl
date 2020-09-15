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
   render_manager::render_manager(gfx::window* const p_wnd,
                                  const std::shared_ptr<util::logger>& p_logger) :
      mp_window{p_wnd},
      mp_logger{p_logger}, m_loader{mp_logger}
   {
      m_instance = create_instance();
      m_device = create_logical_device();
      m_swapchain = create_swapchain();
      m_render_pass = create_swapchain_render_pass();
      m_framebuffers = create_swapchain_framebuffers();
      m_shader_codex = create_shader_codex();
      m_command_pool = create_command_pool();

      m_graphics_pipeline =
         vkn::graphics_pipeline::builder{m_device, m_render_pass, mp_logger}
            .add_shader(m_shader_codex.get_shader("test_shader.vert"))
            .add_shader(m_shader_codex.get_shader("test_shader.frag"))
            .add_vertex_binding({.binding = 0,
                                 .stride = sizeof(::gfx::vertex),
                                 .inputRate = vk::VertexInputRate::eVertex})
            .add_vertex_attribute({.location = 0,
                                   .binding = 0,
                                   .format = vk::Format::eR32G32B32Sfloat,
                                   .offset = offsetof(::gfx::vertex, position)})
            .add_vertex_attribute({.location = 1,
                                   .binding = 0,
                                   .format = vk::Format::eR32G32B32Sfloat,
                                   .offset = offsetof(::gfx::vertex, colour)})
            .add_set_layout("camera_layout",
                            {{.binding = 0,
                              .descriptorType = vk::DescriptorType::eUniformBuffer,
                              .descriptorCount = 1,
                              .stageFlags = vk::ShaderStageFlagBits::eVertex}})
            .add_viewport({.x = 0.0f,
                           .y = 0.0f,
                           .width = static_cast<float>(m_swapchain.extent().width),
                           .height = static_cast<float>(m_swapchain.extent().height),
                           .minDepth = 0.0f,
                           .maxDepth = 1.0f},
                          {.offset = {0, 0}, .extent = m_swapchain.extent()})
            .build()
            .map_error([&](vkn::error&& err) {
               log_error(mp_logger, "[core] Failed to create graphics pipeline: \"{0}\"",
                         err.type.message());

               abort();

               return vkn::graphics_pipeline{};
            })
            .join();

      m_image_available_semaphores = create_image_available_semaphores();
      m_render_finished_semaphores = create_render_finished_semaphores();
      m_in_flight_fences = create_in_flight_fences();
      m_camera_descriptor_pool = create_camera_descriptor_pool();

      m_vertex_buffer = ::gfx::vertex_buffer::make({.vertices = m_triangle_vertices,
                                                    .p_device = &m_device,
                                                    .p_command_pool = &m_command_pool,
                                                    .p_logger = mp_logger})
                           .map_error([&](::gfx::error_t err) {
                              log_error(mp_logger, "[core] Failed to create vertex buffer: \"{0}\"",
                                        err.value().message());
                              std::terminate();

                              return ::gfx::vertex_buffer{};
                           })
                           .join();

      m_index_buffer = ::gfx::index_buffer::make({.indices = m_triangle_indices,
                                                  .p_device = &m_device,
                                                  .p_command_pool = &m_command_pool,
                                                  .p_logger = mp_logger})
                          .map_error([&](::gfx::error_t err) {
                             log_error(mp_logger, "[core] Failed to create vertex buffer: \"{0}\"",
                                       err.value().message());
                             std::terminate();

                             return ::gfx::index_buffer{};
                          })
                          .join();

      m_images_in_flight.resize(std::size(m_swapchain.image_views()), {nullptr});

      util::log_info(mp_logger, "[core] recording main rendering command buffers");
      for (std::size_t i = 0; const auto& buffer : m_command_pool.primary_cmd_buffers())
      {
         buffer.begin({.pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr});

         const auto clear_colour = vk::ClearValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}};
         buffer.beginRenderPass({.pNext = nullptr,
                                 .renderPass = m_render_pass.value(),
                                 .framebuffer = m_framebuffers[i].value(),
                                 .renderArea = {{0, 0}, m_swapchain.extent()},
                                 .clearValueCount = 1,
                                 .pClearValues = &clear_colour},
                                vk::SubpassContents::eInline);

         buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline.value());

         buffer.bindVertexBuffers(0, {m_vertex_buffer->value()}, {vk::DeviceSize{0}});
         buffer.bindIndexBuffer(m_index_buffer->value(), 0, vk::IndexType::eUint32);

         buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline.layout(),
                                   0, {m_camera_descriptor_pool.sets()[i]}, {});
         buffer.drawIndexed(std::size(m_triangle_indices), 1, 0, 0, 0);

         buffer.endRenderPass();
         buffer.end();

         ++i;
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

   auto render_manager::create_instance() const noexcept -> vkn::instance
   {
      return vkn::instance::builder{m_loader, mp_logger}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name(m_engine_name)
         .set_engine_version(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH)
         .build()
         .map_error([&](auto&& err) {
            log_error(mp_logger, "[core] Failed to create instance: {0}", err.type.message());

            std::terminate();

            return vkn::instance{};
         })
         .join();
   }
   auto render_manager::create_physical_device() const noexcept -> vkn::physical_device
   {
      return vkn::physical_device::selector{m_instance, mp_logger}
         .set_surface(mp_window->get_surface(m_instance.value())
                         .map_error([&](auto&& err) {
                            log_error(mp_logger, "[core] Failed to create surface: {0}",
                                      err.type.message());
                            std::terminate();

                            return vk::SurfaceKHR{};
                         })
                         .join())
         .set_preferred_gpu_type(vkn::physical_device::type::discrete)
         .allow_any_gpu_type()
         .require_present()
         .select()
         .map_error([&](auto&& err) {
            log_error(mp_logger, "[core] Failed to create physical device: {0}",
                      err.type.message());
            std::terminate();

            return vkn::physical_device{};
         })
         .join();
   }
   auto render_manager::create_logical_device() const noexcept -> vkn::device
   {
      return vkn::device::builder{m_loader, create_physical_device(), m_instance.version(),
                                  mp_logger}
         .build()
         .map_error([&](auto&& err) {
            log_error(mp_logger, "[core] Failed to create device: {0}", err.type.message());
            std::terminate();

            return vkn::device{};
         })
         .join();
   }
   auto render_manager::create_swapchain() const noexcept -> vkn::swapchain
   {
      return vkn::swapchain::builder{m_device, mp_logger}
         .set_desired_format({vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear})
         .set_desired_present_mode(vk::PresentModeKHR::eMailbox)
         .add_fallback_present_mode(vk::PresentModeKHR::eFifo)
         .set_clipped(true)
         .set_composite_alpha_flags(vk::CompositeAlphaFlagBitsKHR::eOpaque)
         .build()
         .map_error([&](auto&& err) {
            log_error(mp_logger, "[core] Failed to create swapchain: {0}", err.type.message());
            std::terminate();

            return vkn::swapchain{};
         })
         .join();
   }
   auto render_manager::create_swapchain_render_pass() const noexcept -> vkn::render_pass
   {
      return vkn::render_pass::builder{m_device, m_swapchain, mp_logger}
         .build()
         .map_error([&](auto&& err) {
            log_error(mp_logger, "[core] Failed to create render pass: \"{0}\"",
                      err.type.message());
            abort();

            return vkn::render_pass{};
         })
         .join();
   }
   auto render_manager::create_swapchain_framebuffers() const noexcept
      -> util::small_dynamic_array<vkn::framebuffer, vkn::expected_image_count.value()>
   {
      util::small_dynamic_array<vkn::framebuffer, vkn::expected_image_count.value()> framebuffers;
      framebuffers.reserve(std::size(m_swapchain.image_views()));

      for (const auto& img_view : m_swapchain.image_views())
      {
         framebuffers.emplace_back(vkn::framebuffer::builder{m_device, m_render_pass, mp_logger}
                                      .add_attachment(img_view.get())
                                      .set_buffer_width(m_swapchain.extent().width)
                                      .set_buffer_height(m_swapchain.extent().height)
                                      .set_layer_count(1u)
                                      .build()
                                      .map_error([&](vkn::error&& err) {
                                         log_error(mp_logger,
                                                   "[core] Failed to create framebuffer: \"{0}\"",
                                                   err.type.message());
                                         abort();

                                         return vkn::framebuffer{};
                                      })
                                      .join());
      }

      return framebuffers;
   }
   auto render_manager::create_shader_codex() const noexcept -> shader_codex
   {
      return shader_codex::builder{m_device, mp_logger}
         .add_shader_filepath("resources/shaders/test_shader.vert")
         .add_shader_filepath("resources/shaders/test_shader.frag")
         .allow_caching(false)
         .build()
         .map_error([&](error_t&& err) {
            log_error(mp_logger, "[core] Failed to create shader codex: \"{0}\"",
                      err.value().message());
            std::terminate();

            return shader_codex{};
         })
         .join();
   }
   auto render_manager::create_command_pool() const noexcept -> vkn::command_pool
   {
      return vkn::command_pool::builder{m_device, mp_logger}
         .set_queue_family_index(m_device.get_queue_index(vkn::queue::type::graphics)
                                    .map_error([&](auto&& err) {
                                       log_error(mp_logger,
                                                 "[core] No usable graphics queues found: \"{0}\"",
                                                 err.type.message());
                                       std::terminate();

                                       return 0u;
                                    })
                                    .join())
         .set_primary_buffer_count(std::size(m_framebuffers))
         .build()
         .map_error([&](auto&& err) {
            log_error(mp_logger, "[core] Failed to create command pool: \"{0}\"",
                      err.type.message());

            std::terminate();

            return vkn::command_pool{};
         })
         .join();
   }
   auto render_manager::create_camera_descriptor_pool() const noexcept -> vkn::descriptor_pool
   {
      return vkn::descriptor_pool::builder{m_device, mp_logger}
         .add_pool_size(vk::DescriptorType::eUniformBuffer,
                        util::count32_t{std::size(m_swapchain.image_views())})
         .set_descriptor_set_layout(
            vkn::value(m_graphics_pipeline.get_descriptor_set_layout("camera_layout")))
         .set_max_sets(util::count32_t{std::size(m_swapchain.image_views())})
         .build()
         .map_error([&](vkn::error&& err) {
            log_error(mp_logger, "[core] Failed to camera descriptor pool: \"{0}\"",
                      err.type.message());
            std::terminate();

            return vkn::descriptor_pool{};
         })
         .join();
   }

   auto render_manager::create_image_available_semaphores() const noexcept
      -> std::array<vkn::semaphore, max_frames_in_flight>
   {
      std::array<vkn::semaphore, max_frames_in_flight> semaphores;
      for (auto& semaphore : semaphores)
      {
         semaphore = vkn::semaphore::builder{m_device, mp_logger}
                        .build()
                        .map_error([&](vkn::error&& err) {
                           log_error(mp_logger, "[core] Failed to create semaphore: \"{0}\"",
                                     err.type.message());
                           abort();

                           return vkn::semaphore{};
                        })
                        .join();
      }

      return semaphores;
   }
   auto render_manager::create_render_finished_semaphores() const noexcept
      -> std::array<vkn::semaphore, max_frames_in_flight>
   {
      std::array<vkn::semaphore, max_frames_in_flight> semaphores;
      for (auto& semaphore : semaphores)
      {
         semaphore = vkn::semaphore::builder{m_device, mp_logger}
                        .build()
                        .map_error([&](vkn::error&& err) {
                           log_error(mp_logger, "[core] Failed to create semaphore: \"{0}\"",
                                     err.type.message());

                           std::terminate();

                           return vkn::semaphore{};
                        })
                        .join();
      }

      return semaphores;
   }
   auto render_manager::create_in_flight_fences() const noexcept
      -> std::array<vkn::fence, max_frames_in_flight>
   {
      std::array<vkn::fence, max_frames_in_flight> fences;
      for (auto& fence : fences)
      {
         fence = vkn::fence::builder{m_device, mp_logger}
                    .set_signaled()
                    .build()
                    .map_error([&](vkn::error&& err) {
                       log_error(mp_logger, "[core] Failed to create in flight fence: \"{0}\"",
                                 err.type.message());

                       std::terminate();

                       return vkn::fence{};
                    })
                    .join();
      }

      return fences;
   }

} // namespace core
