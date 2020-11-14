#include <water_simulation/render_system.hpp>

#include <gfx/data_types.hpp>

#include <utility>

auto to_util_error(ui::error_t&& err) -> util::error_t
{
   return {err.value()};
}

namespace detail
{
   struct swapchain_data
   {
      vkn::swapchain swapchain{};
      vkn::render_pass render_pass{};
      framebuffer_array framebuffers{};
   };

   auto create_swapchain_render_pass(const vkn::device& device, vkn::swapchain&& swapchain,
                                     const std::shared_ptr<util::logger>& p_logger)
      -> util::result<swapchain_data>
   {
      return vkn::render_pass::builder{device, swapchain, p_logger}.build().map(
         [&](vkn::render_pass pass) {
            return swapchain_data{.swapchain = std::move(swapchain),
                                  .render_pass = std::move(pass)};
         });
   }

   auto create_swapchain_framebuffers(swapchain_data&& data, const vkn::device& device,
                                      const std::shared_ptr<util::logger>& p_logger)
      -> util::result<swapchain_data>
   {
      framebuffer_array framebuffers;
      framebuffers.reserve(std::size(data.swapchain.image_views()));

      for (const auto& img_view : data.swapchain.image_views())
      {
         auto framebuffer_res = vkn::framebuffer::builder{device, data.render_pass, p_logger}
                                   .add_attachment(img_view.get())
                                   .set_buffer_width(data.swapchain.extent().width)
                                   .set_buffer_height(data.swapchain.extent().height)
                                   .set_layer_count(1U)
                                   .build();

         if (auto err = framebuffer_res.error())
         {
            return monad::err(err.value());
         }

         framebuffers.emplace_back(std::move(framebuffer_res).value().value());
      }

      data.framebuffers = std::move(framebuffers);

      return std::move(data);
   }

   struct sync_data
   {
      semaphore_array render_finished_semaphores{};

      std::array<vkn::semaphore, max_frames_in_flight> image_available_semaphores{};
      std::array<vkn::fence, max_frames_in_flight> in_flight_fences{};
   };

   auto create_in_flight_fences(const vkn::device& device,
                                const std::shared_ptr<util::logger>& p_logger)
      -> util::result<std::array<vkn::fence, max_frames_in_flight>>
   {
      std::array<vkn::fence, max_frames_in_flight> fences;
      for (auto& fence : fences)
      {
         if (auto res = vkn::fence::builder{device, p_logger}.set_signaled().build())
         {
            fence = std::move(res).value().value();
         }
         else
         {
            return monad::err(res.error().value());
         }
      }

      return fences;
   }

   auto create_image_available_semaphores(const vkn::device& device,
                                          const std::shared_ptr<util::logger>& p_logger)
      -> util::result<std::array<vkn::semaphore, max_frames_in_flight>>
   {
      std::array<vkn::semaphore, max_frames_in_flight> semaphores;
      for (auto& semaphore : semaphores)
      {
         if (auto res = vkn::semaphore::builder{device, p_logger}.build())
         {
            semaphore = std::move(res).value().value();
         }
         else
         {
            return monad::err(res.error().value());
         }
      }

      return semaphores;
   }
   auto create_render_finished_semaphores(const vkn::device& device,
                                          const vkn::swapchain& swapchain,
                                          const std::shared_ptr<util::logger>& p_logger) noexcept
      -> util::result<sync_data>
   {
      util::small_dynamic_array<vkn::semaphore, vkn::expected_image_count.value()> semaphores;

      for ([[maybe_unused]] const auto& _ : swapchain.image_views())
      {
         if (auto res = vkn::semaphore::builder{device, p_logger}.build())
         {
            semaphores.emplace_back(std::move(res).value().value());
         }
         else
         {
            return monad::err(res.error().value());
         }
      }

      return sync_data{.render_finished_semaphores = std::move(semaphores)};
   }

   /*
   auto render_manager::create_command_pool() const noexcept
      -> std::array<vkn::command_pool, max_frames_in_flight>
   {
      std::array<vkn::command_pool, max_frames_in_flight> pools;

      for (auto& pool : pools)
      {
         pool = vkn::command_pool::builder{m_device, mp_logger}
                   .set_queue_family_index(
                      m_device.get_queue_index(vkn::queue_type::graphics)
                         .map_error([&](auto&& err) {
                            log_error(mp_logger, "[core] No usable graphics queues found: \"{0}\"",
                                      err.value().message());
                            std::terminate();

                            return 0u;
                         })
                         .join())
                   .set_primary_buffer_count(1)
                   .build()
                   .map_error([&](auto&& err) {
                      log_error(mp_logger, "[core] Failed to create command pool: \"{0}\"",
                                err.value().message());

                      std::terminate();

                      return vkn::command_pool{};
                   })
                   .join();
      }

      return pools;
   }
   */
} // namespace detail

auto create_device(ui::window* p_window, const vkn::context& context) -> util::result<vkn::device>;

auto create_swapchain_data(vkn::device& device, const std::shared_ptr<util::logger>& p_logger)
   -> util::result<detail::swapchain_data>;

auto create_sync_primitives(const vkn::device& device, const vkn::swapchain& swapchain,
                            const std::shared_ptr<util::logger>& p_logger)
   -> util::result<detail::sync_data>;

auto create_render_command_pools(const vkn::device& device,
                                 const std::shared_ptr<util::logger>& p_logger)
   -> util::result<std::array<vkn::command_pool, max_frames_in_flight>>;

auto render_system::make(create_info&& info) -> util::result<render_system>
{
   return vkn::context::make({.p_logger = info.p_logger})
      .and_then([&](vkn::context&& ctx) {
         return create_device(info.p_window, ctx).map([&](vkn::device&& device) {
            render_system sys{};
            sys.mp_logger = info.p_logger;
            sys.mp_window = info.p_window;
            sys.m_context = std::move(ctx);
            sys.m_device = std::move(device);

            return sys;
         });
      })
      .and_then([](render_system&& sys) {
         return create_swapchain_data(sys.m_device, sys.mp_logger)
            .map([&](detail::swapchain_data&& data) {
               sys.m_swapchain = std::move(data.swapchain);
               sys.m_swapchain_render_pass = std::move(data.render_pass);
               sys.m_swapchain_framebuffers = std::move(data.framebuffers);
               sys.m_images_in_flight.resize(std::size(sys.m_swapchain.image_views()));
               sys.m_configuration =
                  config{.swapchain_image_count = std::size(sys.m_swapchain.image_views())};

               return std::move(sys);
            });
      })
      .and_then([](render_system&& sys) {
         return create_sync_primitives(sys.m_device, sys.m_swapchain, sys.mp_logger)
            .map([&](detail::sync_data&& data) {
               sys.m_render_finished_semaphores = std::move(data.render_finished_semaphores);
               sys.m_image_available_semaphores = std::move(data.image_available_semaphores);
               sys.m_in_flight_fences = std::move(data.in_flight_fences);
               sys.m_images_in_flight.resize(std::size(sys.m_swapchain.image_views()), {nullptr});

               return std::move(sys);
            });
      })
      .and_then([](render_system&& sys) {
         return create_render_command_pools(sys.m_device, sys.mp_logger).map([&](auto pools) {
            sys.m_render_command_pools = std::move(pools);

            return std::move(sys);
         });
      });
}

auto render_system::begin_frame() -> image_index_t
{
   m_device.logical_device().waitForFences(
      {vkn::value(m_in_flight_fences.at(m_current_frame_index.value()))}, true,
      std::numeric_limits<std::uint64_t>::max());

   auto [image_res, image_index] = m_device.logical_device().acquireNextImageKHR(
      vkn::value(m_swapchain), std::numeric_limits<std::uint64_t>::max(),
      vkn::value(m_image_available_semaphores.at(m_current_frame_index.value())), nullptr);
   if (image_res != vk::Result::eSuccess)
   {
      abort();
   }

   util::log_debug(mp_logger, R"([gfx] swapchain image "{}" acquired)", image_index);

   m_device.logical_device().resetCommandPool(
      m_render_command_pools[m_current_frame_index.value()].value(), {}); // NOLINT

   m_current_image_index = image_index;

   return m_current_image_index;
}

void render_system::record_draw_calls(const std::function<void(vk::CommandBuffer)>& buffer_calls)
{
   util::log_debug(mp_logger, R"([gfx] render command pool "{}" buffer recording)",
                   m_current_frame_index.value());

   for (const auto& buffer :
        m_render_command_pools[m_current_frame_index.value()].primary_cmd_buffers()) // NOLINT
   {
      buffer.begin({.pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr});

      const auto clear_colour = vk::ClearValue{std::array<float, 4>{0.0F, 0.0F, 0.0F, 0.0F}};
      buffer.beginRenderPass(
         {.pNext = nullptr,
          .renderPass = m_swapchain_render_pass.value(),
          .framebuffer = m_swapchain_framebuffers[m_current_image_index.value()].value(),
          .renderArea = {{0, 0}, m_swapchain.extent()},
          .clearValueCount = 1,
          .pClearValues = &clear_colour},
         vk::SubpassContents::eInline);

      std::invoke(buffer_calls, buffer);

      buffer.endRenderPass();
      buffer.end();
   }
}

void render_system::end_frame()
{
   if (m_images_in_flight[m_current_image_index.value()])
   {
      m_device.logical_device().waitForFences(
         {vkn::value(m_images_in_flight.at(m_current_frame_index.value()))}, true,
         std::numeric_limits<std::uint64_t>::max());
   }
   m_images_in_flight[m_current_image_index.value()] =
      vkn::value(m_in_flight_fences.at(m_current_frame_index.value()));

   const std::array wait_semaphores{
      vkn::value(m_image_available_semaphores.at(m_current_frame_index.value()))};
   const std::array signal_semaphores{
      vkn::value(m_render_finished_semaphores.at(m_current_image_index.value()))};
   const std::array command_buffers{
      m_render_command_pools[m_current_frame_index.value()].primary_cmd_buffers()[0]}; // NOLINT
   const std::array<vk::PipelineStageFlags, 1> wait_stages{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

   m_device.logical_device().resetFences(
      {vkn::value(m_in_flight_fences.at(m_current_frame_index.value()))});

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
      const auto gfx_queue = *m_device.get_queue(vkn::queue_type::graphics).value();
      gfx_queue.submit(submit_infos,
                       vkn::value(m_in_flight_fences.at(m_current_frame_index.value())));
   }
   catch (const vk::SystemError& err)
   {
      util::log_error(mp_logger, "[core] failed to submit graphics queue");
      abort();
   }

   const std::array swapchains{vkn::value(m_swapchain)};

   const auto present_queue = *m_device.get_queue(vkn::queue_type::present).value();
   if (present_queue.presentKHR({.waitSemaphoreCount = std::size(signal_semaphores),
                                 .pWaitSemaphores = std::data(signal_semaphores),
                                 .swapchainCount = std::size(swapchains),
                                 .pSwapchains = std::data(swapchains),
                                 .pImageIndices = &m_current_image_index.value()}) !=
       vk::Result::eSuccess)
   {
      util::log_error(mp_logger, "[core] failed to present present queue");
      abort();
   }

   m_current_frame_index = (m_current_frame_index + 1) % max_frames_in_flight;
}

void render_system::wait()
{
   m_device.logical_device().waitIdle();
}

auto render_system::device() -> vkn::device&
{
   return m_device;
}
auto render_system::render_pass() -> vkn::render_pass&
{
   return m_swapchain_render_pass;
}

auto render_system::vertex_bindings() -> vertex_bindings_array
{
   return {
      {.binding = 0, .stride = sizeof(gfx::vertex), .inputRate = vk::VertexInputRate::eVertex}};
}
auto render_system::vertex_attributes() -> vertex_attributes_array
{
   return {{.location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(gfx::vertex, position)},
           {.location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(gfx::vertex, normal)},
           {.location = 2,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(gfx::vertex, colour)}};
}

auto render_system::viewport() const -> vk::Viewport
{
   return {.x = 0.0F,
           .y = 0.0F,
           .width = static_cast<float>(m_swapchain.extent().width),
           .height = static_cast<float>(m_swapchain.extent().height),
           .minDepth = 0.0F,
           .maxDepth = 1.0F};
}
auto render_system::scissor() const -> vk::Rect2D
{
   return {.offset = {0, 0}, .extent = m_swapchain.extent()};
}

auto render_system::create_vertex_buffer(const util::dynamic_array<gfx::vertex>& vertices) const
   -> util::result<gfx::vertex_buffer>
{
   return gfx::vertex_buffer::make({.vertices = vertices,
                                    .device = m_device,
                                    .command_pool = m_render_command_pools[0],
                                    .p_logger = mp_logger});
}
auto render_system::create_index_buffer(const util::dynamic_array<std::uint32_t>& indices) const
   -> util::result<gfx::index_buffer>
{
   return gfx::index_buffer::make({.indices = indices,
                                   .device = m_device,
                                   .command_pool = m_render_command_pools[0],
                                   .p_logger = mp_logger});
}

auto render_system::lookup_configuration() const -> const config&
{
   return m_configuration;
}

auto create_device(ui::window* p_window, const vkn::context& context) -> util::result<vkn::device>
{
   return p_window->get_surface(context.instance())
      .map_error(to_util_error)
      .and_then([&](vk::UniqueSurfaceKHR&& surface) {
         return context.select_device(std::move(surface));
      });
}

auto create_swapchain_data(vkn::device& device, const std::shared_ptr<util::logger>& p_logger)
   -> util::result<detail::swapchain_data>
{
   return vkn::swapchain::builder{device, p_logger}
      .set_desired_format({vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear})
      .set_desired_present_mode(vk::PresentModeKHR::eMailbox)
      .add_fallback_present_mode(vk::PresentModeKHR::eFifo)
      .set_clipped(true)
      .set_composite_alpha_flags(vk::CompositeAlphaFlagBitsKHR::eOpaque)
      .build()
      .and_then([&](vkn::swapchain&& swap) {
         return detail::create_swapchain_render_pass(device, std::move(swap), p_logger);
      })
      .and_then([&](detail::swapchain_data&& data) {
         return create_swapchain_framebuffers(std::move(data), device, p_logger);
      });
}

auto create_sync_primitives(const vkn::device& device, const vkn::swapchain& swapchain,
                            const std::shared_ptr<util::logger>& p_logger)
   -> util::result<detail::sync_data>
{
   return detail::create_render_finished_semaphores(device, swapchain, p_logger)
      .and_then([&](detail::sync_data&& data) {
         return detail::create_image_available_semaphores(device, p_logger).map([&](auto sem) {
            data.image_available_semaphores = std::move(sem);

            return std::move(data);
         });
      })
      .and_then([&](detail::sync_data&& data) {
         return detail::create_in_flight_fences(device, p_logger).map([&](auto fences) {
            data.in_flight_fences = std::move(fences);

            return std::move(data);
         });
      });
}

auto create_render_command_pools(const vkn::device& device,
                                 const std::shared_ptr<util::logger>& p_logger)
   -> util::result<std::array<vkn::command_pool, max_frames_in_flight>>
{
   std::array<vkn::command_pool, max_frames_in_flight> pools;

   for (auto& pool : pools)
   {
      auto result =
         device.get_queue_index(vkn::queue_type::graphics).and_then([&](std::uint32_t i) {
            return vkn::command_pool::builder{device, p_logger}
               .set_queue_family_index(i)
               .set_primary_buffer_count(1U)
               .build();
         });

      if (auto err = result.error())
      {
         return monad::err(err.value());
      }

      pool = std::move(result).value().value();
   }

   return pools;
}
