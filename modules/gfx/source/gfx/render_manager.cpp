#include <gfx/memory/index_buffer.hpp>
#include <gfx/render_manager.hpp>

#include <gfx/data_types.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace gfx
{
   render_manager::render_manager(const context& ctx, const window& wnd,
                                  std::shared_ptr<util::logger> p_logger) :
      mp_logger{std::move(p_logger)},
      m_ctx{ctx}, m_wnd{wnd}
   {
      m_device = create_logical_device();
      m_swapchain = create_swapchain();
      m_swapchain_render_pass = create_swapchain_render_pass();
      m_swapchain_framebuffers = create_swapchain_framebuffers();

      // Sync variables
      m_image_available_semaphores = create_image_available_semaphores();
      m_render_finished_semaphores = create_render_finished_semaphores();
      m_in_flight_fences = create_in_flight_fences();

      m_shader_codex = create_shader_codex();

      m_gfx_command_pools = create_command_pool();

      m_images_in_flight.resize(std::size(m_swapchain.image_views()), {nullptr});
   }

   auto render_manager::subscribe_renderable(const std::string& name, const renderable_data& r)
      -> bool
   {
      auto vertex = vertex_buffer::make({.vertices = r.vertices,
                                         .p_device = &m_device,
                                         .p_command_pool = &m_gfx_command_pools[0],
                                         .p_logger = mp_logger});

      auto index = index_buffer::make({.indices = r.indices,
                                       .p_device = &m_device,
                                       .p_command_pool = &m_gfx_command_pools[0],
                                       .p_logger = mp_logger});

      if (!(vertex && index))
      {
         return false;
      }

      m_renderables_to_index.insert_or_assign(name, std::size(m_renderables));
      m_renderable_model_matrices.emplace_back(r.model);
      m_renderables.emplace_back(renderable{.name = name,
                                            .vertex_buffer = std::move(vertex).value().value(),
                                            .index_buffer = std::move(index).value().value()});

      return true;
   }

   void render_manager::update_model_matrix(const std::string& name, const glm::mat4& model)
   {
      auto it = m_renderables_to_index.find(name);
      if (it != m_renderables_to_index.end())
      {
         m_renderable_model_matrices[it->second] = model;
      }
      else
      {
         util::log_warn(mp_logger, "[gfx] failed to update model matrix for renderable \"{}\"",
                        name);
      }
   }

   void render_manager::bake()
   {
      m_graphics_pipeline =
         vkn::graphics_pipeline::builder{m_device, m_swapchain_render_pass, mp_logger}
            .add_shader(m_shader_codex.get_shader("test_shader.vert"))
            .add_shader(m_shader_codex.get_shader("test_shader.frag"))
            .add_vertex_binding({.binding = 0,
                                 .stride = sizeof(gfx::vertex),
                                 .inputRate = vk::VertexInputRate::eVertex})
            .add_vertex_attribute({.location = 0,
                                   .binding = 0,
                                   .format = vk::Format::eR32G32B32Sfloat,
                                   .offset = offsetof(gfx::vertex, position)})
            .add_vertex_attribute({.location = 1,
                                   .binding = 0,
                                   .format = vk::Format::eR32G32B32Sfloat,
                                   .offset = offsetof(gfx::vertex, colour)})
            .add_set_layout("camera_layout",
                            {{.binding = 0,
                              .descriptorType = vk::DescriptorType::eUniformBuffer,
                              .descriptorCount = 1,
                              .stageFlags = vk::ShaderStageFlagBits::eVertex}})
            .add_push_constant(
               "mesh_data", vkn::shader_type::vertex, util::size_t{0},
               util::size_t{sizeof(glm::mat4) * std::size(m_renderable_model_matrices)})
            .add_viewport({.x = 0.0F,
                           .y = 0.0F,
                           .width = static_cast<float>(m_swapchain.extent().width),
                           .height = static_cast<float>(m_swapchain.extent().height),
                           .minDepth = 0.0F,
                           .maxDepth = 1.0F},
                          {.offset = {0, 0}, .extent = m_swapchain.extent()})
            .build()
            .map_error([&](vkn::error&& err) {
               log_error(mp_logger, "[core] Failed to create graphics pipeline: \"{0}\"",
                         err.type.message());

               abort();

               return vkn::graphics_pipeline{};
            })
            .join();

      m_camera_descriptor_pool = create_camera_descriptor_pool();
      m_camera_buffers = create_camera_buffers();

      for (std::size_t i = 0; auto set : m_camera_descriptor_pool.sets())
      {
         std::array buf_info{vk::DescriptorBufferInfo{.buffer = vkn::value(*m_camera_buffers[i++]),
                                                      .offset = 0,
                                                      .range = sizeof(gfx::camera_matrices)}};
         vk::WriteDescriptorSet write{.dstSet = set,
                                      .dstBinding = 0,
                                      .dstArrayElement = 0,
                                      .descriptorCount = std::size(buf_info),
                                      .descriptorType = vk::DescriptorType::eUniformBuffer,
                                      .pBufferInfo = std::data(buf_info)};

         m_device->updateDescriptorSets({write}, {});
      }
   }

   void render_manager::render_frame()
   {
      m_device->waitForFences({vkn::value(m_in_flight_fences.at(m_current_frame))}, true,
                              std::numeric_limits<std::uint64_t>::max());

      auto [image_res, image_index] = m_device->acquireNextImageKHR(
         vkn::value(m_swapchain), std::numeric_limits<std::uint64_t>::max(),
         vkn::value(m_image_available_semaphores.at(m_current_frame)), nullptr);
      if (image_res != vk::Result::eSuccess)
      {
         abort();
      }

      util::log_info(mp_logger, "[core] recording main rendering command buffers");

      m_device->resetCommandPool(m_gfx_command_pools[m_current_frame].value(), {});
      for (const auto& buffer : m_gfx_command_pools[m_current_frame].primary_cmd_buffers())
      {
         buffer.begin({.pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr});

         const auto clear_colour = vk::ClearValue{std::array<float, 4>{0.0F, 0.0F, 0.0F, 0.0F}};
         buffer.beginRenderPass({.pNext = nullptr,
                                 .renderPass = m_swapchain_render_pass.value(),
                                 .framebuffer = m_swapchain_framebuffers[image_index].value(),
                                 .renderArea = {{0, 0}, m_swapchain.extent()},
                                 .clearValueCount = 1,
                                 .pClearValues = &clear_colour},
                                vk::SubpassContents::eInline);

         buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline.value());
         buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline.layout(),
                                   0, {m_camera_descriptor_pool.sets()[image_index]}, {});

         for (const auto& [name, index] : m_renderables_to_index)
         {
            util::log_debug(mp_logger, R"([gfx] buffer calls for renderable "{}" at index "{}")",
                            name, index);

            buffer.pushConstants(
               m_graphics_pipeline.layout(),
               m_graphics_pipeline.get_push_constant_ranges("mesh_data").stageFlags,
               sizeof(glm::mat4) * index, sizeof(glm::mat4) * 1,
               &m_renderable_model_matrices[index]);
            buffer.bindVertexBuffers(0, {m_renderables[index].vertex_buffer->value()},
                                     {vk::DeviceSize{0}});
            buffer.bindIndexBuffer(m_renderables[index].index_buffer->value(), 0,
                                   vk::IndexType::eUint32);
            buffer.drawIndexed(m_renderables[index].index_buffer.index_count(), 1, 0, 0, 0);
         }

         buffer.endRenderPass();
         buffer.end();
      }

      update_camera(image_index);

      if (m_images_in_flight[image_index])
      {
         m_device->waitForFences({vkn::value(m_images_in_flight.at(m_current_frame))}, true,
                                 std::numeric_limits<std::uint64_t>::max());
      }
      m_images_in_flight[image_index] = vkn::value(m_in_flight_fences.at(m_current_frame));

      const std::array wait_semaphores{
         vkn::value(m_image_available_semaphores.at(m_current_frame))};
      const std::array signal_semaphores{vkn::value(m_render_finished_semaphores.at(image_index))};
      const std::array command_buffers{
         m_gfx_command_pools[m_current_frame].primary_cmd_buffers()[0]};
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

   void render_manager::update_camera(uint32_t image_index)
   {
      gfx::camera_matrices matrices{};
      matrices.perspective = glm::perspective(
         glm::radians(45.0F), m_swapchain.extent().width / (float)m_swapchain.extent().height, 0.1F,
         10.0F);
      matrices.view = glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                  glm::vec3(0.0F, 0.0F, 1.0F));
      matrices.perspective[1][1] *= -1;

      void* p_data =
         m_device->mapMemory(m_camera_buffers[image_index]->memory(), 0, sizeof(matrices), {});
      memcpy(p_data, &matrices, sizeof(matrices));
      m_device->unmapMemory(m_camera_buffers[image_index]->memory());
   }

   auto render_manager::add_pass(const std::string& name,
                                 [[maybe_unused]] vkn::queue::type queue_type) -> render_pass&
   {
      auto it = m_render_pass_to_index.find(name);
      if (it != std::end(m_render_pass_to_index))
      {
         return m_render_passes[it->second.value()];
      }
      else
      {
         const auto index = std::size(m_render_passes);
         m_render_passes.emplace_back(render_pass{this});
         m_render_pass_to_index[name] = index;
         return m_render_passes.back();
      }
   }

   auto render_manager::create_physical_device() const noexcept -> vkn::physical_device
   {
      return vkn::physical_device::selector{m_ctx.vulkan_instance(), mp_logger}
         .set_surface(m_wnd.get_surface(m_ctx.vulkan_instance().value())
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
      return vkn::device::builder{m_ctx.vulkan_loader(), create_physical_device(),
                                  m_ctx.vulkan_instance().version(), mp_logger}
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
         framebuffers.emplace_back(
            vkn::framebuffer::builder{m_device, m_swapchain_render_pass, mp_logger}
               .add_attachment(img_view.get())
               .set_buffer_width(m_swapchain.extent().width)
               .set_buffer_height(m_swapchain.extent().height)
               .set_layer_count(1U)
               .build()
               .map_error([&](vkn::error&& err) {
                  log_error(mp_logger, "[core] Failed to create framebuffer: \"{0}\"",
                            err.type.message());
                  abort();

                  return vkn::framebuffer{};
               })
               .join());
      }

      return framebuffers;
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
   auto render_manager::create_camera_buffers() const noexcept
      -> util::dynamic_array<gfx::camera_buffer>
   {
      const std::size_t buf_count = std::size(m_swapchain.image_views());

      util::dynamic_array<gfx::camera_buffer> buffers{};
      buffers.reserve(buf_count);

      // for ([[maybe_unused]] std::size_t i : std::views::iota(buf_count))
      for (std::size_t i = 0; i < buf_count; ++i)
      {
         if (auto res = gfx::camera_buffer::make({.p_device = &m_device, .p_logger = mp_logger}))
         {
            buffers.emplace_back(std::move(res).value().value());
         }
         else
         {
            // TODO: handle error

            return util::dynamic_array<gfx::camera_buffer>{};
         }
      }

      return buffers;
   }

   auto render_manager::create_shader_codex() const noexcept -> core::shader_codex
   {
      return core::shader_codex::builder{m_device, mp_logger}
         .add_shader_filepath("resources/shaders/test_shader.vert")
         .add_shader_filepath("resources/shaders/test_shader.frag")
         .allow_caching(false)
         .build()
         .map_error([&](auto&& err) {
            log_error(mp_logger, "[core] Failed to create shader codex: \"{0}\"",
                      err.value().message());
            std::terminate();

            return core::shader_codex{};
         })
         .join();
   }

   auto render_manager::create_command_pool() const noexcept
      -> std::array<vkn::command_pool, max_frames_in_flight>
   {
      std::array<vkn::command_pool, max_frames_in_flight> pools;

      for (auto& pool : pools)
      {
         pool = vkn::command_pool::builder{m_device, mp_logger}
                   .set_queue_family_index(
                      m_device.get_queue_index(vkn::queue::type::graphics)
                         .map_error([&](auto&& err) {
                            log_error(mp_logger, "[core] No usable graphics queues found: \"{0}\"",
                                      err.type.message());
                            std::terminate();

                            return 0u;
                         })
                         .join())
                   .set_primary_buffer_count(1)
                   .build()
                   .map_error([&](auto&& err) {
                      log_error(mp_logger, "[core] Failed to create command pool: \"{0}\"",
                                err.type.message());

                      std::terminate();

                      return vkn::command_pool{};
                   })
                   .join();
      }

      return pools;
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
      -> util::small_dynamic_array<vkn::semaphore, vkn::expected_image_count.value()>
   {
      util::small_dynamic_array<vkn::semaphore, vkn::expected_image_count.value()> semaphores;

      for ([[maybe_unused]] const auto& _ : m_swapchain.image_views())
      {
         semaphores.emplace_back(vkn::semaphore::builder{m_device, mp_logger}
                                    .build()
                                    .map_error([&](vkn::error&& err) {
                                       log_error(mp_logger,
                                                 "[core] Failed to create semaphore: \"{0}\"",
                                                 err.type.message());

                                       std::terminate();

                                       return vkn::semaphore{};
                                    })
                                    .join());
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
} // namespace gfx
