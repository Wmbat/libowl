#include <sph-simulation/render/render_system.hpp>

#include <libcacao/command_pool.hpp>
#include <libcacao/device.hpp>

#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <utility>

using namespace reglisse;

auto create_render_command_pools_t(const cacao::device& device, mannele::log_ptr logger)
   -> std::array<cacao::command_pool, max_frames_in_flight_t>
{
   std::array<cacao::command_pool, max_frames_in_flight_t> pools;

   for (auto& pool : pools)
   {
      const std::uint32_t family_index =
         device.get_queue_index(cacao::queue_flag_bits::graphics | cacao::queue_flag_bits::present);

      pool = cacao::command_pool(
         cacao::command_pool_create_info{.device = device,
                                         .queue_family_index = some(family_index),
                                         .primary_buffer_count = 1,
                                         .logger = logger});
   }

   return pools;
}

auto create_in_flight_fences_t(const cacao::device& device)
   -> std::array<vk::UniqueFence, max_frames_in_flight_t>
{
   std::array<vk::UniqueFence, max_frames_in_flight_t> fences;
   for (auto& fence : fences)
   {
      fence = device.logical().createFenceUnique({.flags = vk::FenceCreateFlagBits::eSignaled});
   }

   return fences;
}

auto create_image_available_semaphores_t(const cacao::device& device)
   -> std::array<vk::UniqueSemaphore, max_frames_in_flight_t>
{
   std::array<vk::UniqueSemaphore, max_frames_in_flight_t> semaphores;

   for (auto& semaphore : semaphores)
   {
      semaphore = device.logical().createSemaphoreUnique({});
   }

   return semaphores;
}

auto create_render_finished_semaphores_t(const cacao::device& device, mannele::u64 count)
   -> std::vector<vk::UniqueSemaphore>
{
   std::vector<vk::UniqueSemaphore> semaphores;

   for ([[maybe_unused]] auto i : ranges::views::iota(0u, count))
   {
      semaphores.push_back(device.logical().createSemaphoreUnique({}));
   }

   return semaphores;
}

auto create_depth_buffer(mannele::log_ptr logger, cacao::device& device, vk::Extent2D extent)
   -> image
{
   return image({.device = device,
                 .formats = {std::begin(depth_formats), std::end(depth_formats)},
                 .tiling = vk::ImageTiling::eOptimal,
                 .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                 .memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                 .dimensions = {extent.width, extent.height},
                 .logger = logger});
}

render_system::render_system(cacao::window* p_window, mannele::log_ptr logger) :
   m_logger(logger), mp_window(p_window),
   m_context(
      {.min_vulkan_version = VK_MAKE_VERSION(1, 0, 0), .use_window = true, .logger = m_logger}),
   m_surface(p_window->create_surface(m_context).take()),
   m_device({.ctx = m_context, .surface = some(m_surface.get()), .logger = m_logger}),
   m_swapchain(cacao::swapchain_create_info{
      .device = m_device,
      .surface = m_surface.get(),
      .desired_formats = {{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}},
      .desired_present_modes = {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo},
      .desired_dimensions = p_window->dimension(),
      .image_usage_flags = vk::ImageUsageFlagBits::eColorAttachment,
      .composite_alpha_flags = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .should_clip = true,
      .old_swapchain = nullptr,
      .logger = m_logger}),
   m_render_finished_semaphores(
      create_render_finished_semaphores_t(m_device, std::size(m_swapchain.image_views()))),
   m_render_command_pools(create_render_command_pools_t(m_device, m_logger)),
   m_image_available_semaphores(create_image_available_semaphores_t(m_device)),
   m_in_flight_fences(create_in_flight_fences_t(m_device)),
   m_depth_image(create_depth_buffer(m_logger, m_device, m_swapchain.extent()))
{
   m_images_in_flight.resize(std::size(m_swapchain.images()));
}

auto render_system::begin_frame() -> mannele::u32
{
   [[maybe_unused]] auto _ =
      m_device.logical().waitForFences({m_in_flight_fences.at(m_current_frame_index).get()}, true,
                                       std::numeric_limits<std::uint64_t>::max());

   auto [image_res, image_index] = m_device.logical().acquireNextImageKHR(
      m_swapchain.value(), std::numeric_limits<std::uint64_t>::max(),
      m_image_available_semaphores.at(m_current_frame_index).get(), nullptr);
   if (image_res != vk::Result::eSuccess)
   {
      abort();
   }

   m_logger.debug(R"([gfx] swapchain image "{}" acquired)", image_index);

   m_device.logical().resetCommandPool(m_render_command_pools.at(m_current_frame_index).value(),
                                       {});

   m_current_image_index = image_index;

   return m_current_image_index;
}

void render_system::render(std::span<render_pass> passes)
{
   m_logger.debug(R"([gfx] render command pool "{}" buffer recording)", m_current_frame_index);

   std::array<vk::ClearValue, 2> clear_values{};
   clear_values[0].color = {std::array{0.0F, 0.0F, 0.0F, 0.0F}};
   clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

   for (const auto& buffer :
        m_render_command_pools[m_current_frame_index].primary_buffers()) // NOLINT
   {
      buffer.begin({.pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr});

      for (auto& pass : passes)
      {
         pass.submit_render_calls(buffer, m_current_image_index, {{0, 0}, m_swapchain.extent()},
                                  clear_values);
      }

      buffer.end();
   }
}

void render_system::end_frame()
{
   if (m_images_in_flight.at(m_current_image_index))
   {
      [[maybe_unused]] auto _ =
         m_device.logical().waitForFences({m_images_in_flight.at(m_current_frame_index)}, true,
                                          std::numeric_limits<std::uint64_t>::max());
   }
   m_images_in_flight.at(m_current_image_index) =
      m_in_flight_fences.at(m_current_frame_index).get();

   const std::array wait_semaphores{m_image_available_semaphores.at(m_current_frame_index).get()};
   const std::array signal_semaphores{m_render_finished_semaphores.at(m_current_image_index).get()};
   const std::array command_buffers{
      m_render_command_pools.at(m_current_frame_index).primary_buffers()[0]};
   const std::array<vk::PipelineStageFlags, 1> wait_stages{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

   m_device.logical().resetFences({m_in_flight_fences.at(m_current_frame_index).get()});

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
      const auto gfx_queue = m_device.get_queue(cacao::queue_flag_bits::graphics).value;
      gfx_queue.submit(submit_infos, m_in_flight_fences.at(m_current_frame_index).get());
   }
   catch (const vk::SystemError& err)
   {
      m_logger.error("[gfx] failed to submit graphics queue");

      std::terminate();
   }

   const std::array swapchains{m_swapchain.value()};

   const auto present_queue = m_device.get_queue(cacao::queue_flag_bits::present);
   if (present_queue.value.presentKHR(
          vk::PresentInfoKHR{.waitSemaphoreCount = std::size(signal_semaphores),
                             .pWaitSemaphores = std::data(signal_semaphores),
                             .swapchainCount = std::size(swapchains),
                             .pSwapchains = std::data(swapchains),
                             .pImageIndices = &m_current_image_index}) != vk::Result::eSuccess)
   {
      m_logger.error("[gfx] failed to present present queue");

      std::terminate();
   }

   m_current_frame_index = (m_current_frame_index + 1) % max_frames_in_flight_t;
}

void render_system::wait()
{
   m_device.logical().waitIdle();
}

auto render_system::device() const -> const cacao::device&
{
   return m_device;
}
auto render_system::device() -> cacao::device&
{
   return m_device;
}
auto render_system::swapchain() const -> const cacao::swapchain&
{
   return m_swapchain;
}
auto render_system::swapchain() -> cacao::swapchain&
{
   return m_swapchain;
}

auto render_system::get_depth_attachment() const -> vk::ImageView
{
   return m_depth_image.view();
}

auto render_system::vertex_bindings() -> vertex_bindings_array
{
   return {{.binding = 0, .stride = sizeof(vertex), .inputRate = vk::VertexInputRate::eVertex}};
}
auto render_system::vertex_attributes() -> vertex_attributes_array
{
   return {{.location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(vertex, position)},
           {.location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(vertex, normal)},
           {.location = 2,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(vertex, colour)}};
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

auto render_system::create_vertex_buffer(std::span<const vertex> vertices) const -> vertex_buffer
{
   return vertex_buffer({.device = m_device,
                         .pool = m_render_command_pools[0],
                         .vertices = vertices,
                         .logger = m_logger});
}
auto render_system::create_index_buffer(std::span<const std::uint32_t> indices) const
   -> index_buffer
{
   return index_buffer({.device = m_device,
                        .pool = m_render_command_pools[0],
                        .indices = indices,
                        .logger = m_logger});
}
