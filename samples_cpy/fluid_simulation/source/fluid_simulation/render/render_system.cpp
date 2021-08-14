#include <fluid_simulation/render/render_system.hpp>

#include <cacao/device.hpp>
#include <cacao/vulkan/command_pool.hpp>

#include <utility>

#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

auto to_util_error(ui::error_t&& err) -> util::error_t
{
   return {err.value()};
}

struct render_system_data
{
   render_system::create_info& info;

   cacao::context context{};
   vk::UniqueSurfaceKHR surface{};
   cacao::device device{};
   vkn::swapchain swapchain{};

   semaphore_array render_finished_semaphores{};

   std::array<vkn::command_pool, max_frames_in_flight> render_command_pools{};
   std::array<vk::UniqueSemaphore, max_frames_in_flight> image_available_semaphores{};
   std::array<vk::UniqueFence, max_frames_in_flight> in_flight_fences{};

   cacao::image depth_image{};

   ui::window* p_window{};

   util::logger_wrapper logger{};
};

auto create_context(render_system_data&& data) -> result<render_system_data>
{
   data.context = cacao::context{
      {.min_vulkan_version = VK_MAKE_VERSION(1, 0, 0), .use_window = true, .logger = data.logger}};

   return std::move(data);
}

auto create_device(render_system_data&& data) -> result<render_system_data>
{
   return data.p_window->get_surface(data.context.instance())
      .map_error(to_util_error)
      .map([&](vk::UniqueSurfaceKHR&& surface) {
         data.surface = std::move(surface);
         data.device = cacao::device{
            {.ctx = data.context, .surface = data.surface.get(), .logger = data.logger}};

         return std::move(data);
      });
}

auto create_swapchain(render_system_data&& data) -> result<render_system_data>
{
   return vkn::swapchain::builder{data.device, data.surface.get(), data.logger}
      .set_desired_format({vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear})
      .set_desired_present_mode(vk::PresentModeKHR::eMailbox)
      .add_fallback_present_mode(vk::PresentModeKHR::eFifo)
      .set_clipped(true)
      .set_composite_alpha_flags(vk::CompositeAlphaFlagBitsKHR::eOpaque)
      .build()
      .map([&](vkn::swapchain&& swap) {
         data.swapchain = std::move(swap);

         return std::move(data);
      });
}

auto create_render_command_pools(render_system_data&& data) -> result<render_system_data>
{
   std::array<vkn::command_pool, max_frames_in_flight> pools;

   for (auto& pool : pools)
   {
      const std::uint32_t family_index = data.device.get_queue_index(
         cacao::queue_flag_bits::graphics | cacao::queue_flag_bits::present);

      auto result = vkn::command_pool::builder{data.device, data.logger}
                       .set_queue_family_index(family_index)
                       .set_primary_buffer_count(1U)
                       .build();

      if (auto err = result.error())
      {
         return monad::err(err.value());
      }

      pool = std::move(result).value().value();
   }

   data.render_command_pools = std::move(pools);

   return std::move(data);
}

auto create_in_flight_fences(const cacao::device& device, util::logger_wrapper logger)
   -> std::array<vk::UniqueFence, max_frames_in_flight>
{
   std::array<vk::UniqueFence, max_frames_in_flight> fences;
   for (auto& fence : fences)
   {
      fence = device.logical().createFenceUnique(
         vk::FenceCreateInfo{}.setFlags(vk::FenceCreateFlagBits::eSignaled));

      logger.info("signaled fence created");
   }

   return fences;
}

auto create_image_available_semaphores(const cacao::device& device, util::logger_wrapper logger)
   -> std::array<vk::UniqueSemaphore, max_frames_in_flight>
{
   std::array<vk::UniqueSemaphore, max_frames_in_flight> semaphores;

   for (auto& semaphore : semaphores)
   {
      semaphore = device.logical().createSemaphoreUnique({});

      logger.info("Semaphore created");
   }

   return semaphores;
}

auto create_render_finished_semaphores(const cacao::device& device, cacao::count32_t count,
                                       util::logger_wrapper logger)
   -> crl::small_dynamic_array<vk::UniqueSemaphore, vkn::expected_image_count.value()>
{
   crl::small_dynamic_array<vk::UniqueSemaphore, vkn::expected_image_count.value()> semaphores;

   for ([[maybe_unused]] auto i : ranges::views::iota(0u, count.value()))
   {
      semaphores.append(device.logical().createSemaphoreUnique({}));

      logger.info("Semaphore created");
   }

   return semaphores;
}

auto create_depth_buffer(util::logger_wrapper logger, cacao::device& device, vk::Extent2D extent)
   -> cacao::image
{
   return cacao::image{
      {.logger = logger,
       .device = device,
       .formats = {std::begin(cacao::depth_formats), std::end(cacao::depth_formats)},
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
       .memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
       .width = extent.width,
       .height = extent.height}};
}

auto render_system::make(create_info&& info) -> util::result<render_system>
{
   return create_context({.info = info, .p_window = info.p_window, .logger = info.logger})
      .and_then(create_device)
      .and_then(create_swapchain)
      .and_then(create_render_command_pools)
      .map([](render_system_data&& data) {
         render_system rs{};
         rs.m_logger = data.logger;
         rs.mp_window = data.p_window;
         rs.m_context = std::move(data.context);
         rs.m_device = std::move(data.device);
         rs.m_surface = std::move(data.surface);
         rs.m_swapchain = std::move(data.swapchain);
         rs.m_in_flight_fences = create_in_flight_fences(rs.m_device, rs.m_logger);
         rs.m_image_available_semaphores =
            create_image_available_semaphores(rs.device(), rs.m_logger);
         rs.m_render_finished_semaphores = create_render_finished_semaphores(
            rs.m_device,
            cacao::count32_t{static_cast<std::uint32_t>(std::size(rs.m_swapchain.image_views()))},
            rs.m_logger);
         rs.m_render_command_pools = std::move(data.render_command_pools);
         rs.m_depth_image = create_depth_buffer(rs.m_logger, rs.m_device, rs.m_swapchain.extent());
         rs.m_configuration = {.swapchain_image_count = static_cast<std::uint32_t>(
                                  std::size(rs.m_swapchain.image_views()))};

         rs.m_images_in_flight.resize(std::size(rs.m_swapchain.image_views()));

         return rs;
      });
}

auto render_system::begin_frame() -> image_index_t
{
   m_device.logical().waitForFences({m_in_flight_fences.at(m_current_frame_index.value()).get()},
                                    true, std::numeric_limits<std::uint64_t>::max());

   auto [image_res, image_index] = m_device.logical().acquireNextImageKHR(
      vkn::value(m_swapchain), std::numeric_limits<std::uint64_t>::max(),
      m_image_available_semaphores.at(m_current_frame_index.value()).get(), nullptr);
   if (image_res != vk::Result::eSuccess)
   {
      abort();
   }

   m_logger.debug(R"([gfx] swapchain image "{}" acquired)", image_index);

   m_device.logical().resetCommandPool(
      m_render_command_pools[m_current_frame_index.value()].value(), {}); // NOLINT

   m_current_image_index = image_index;

   return m_current_image_index;
}

void render_system::render(std::span<render_pass> passes)
{
   m_logger.debug(R"([gfx] render command pool "{}" buffer recording)",
                  m_current_frame_index.value());

   std::array<vk::ClearValue, 2> clear_values{};
   clear_values[0].color = {std::array{0.0F, 0.0F, 0.0F, 0.0F}};
   clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

   for (const auto& buffer :
        m_render_command_pools[m_current_frame_index.value()].primary_cmd_buffers()) // NOLINT
   {
      buffer.begin({.pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr});

      for (auto& pass : passes)
      {
         pass.submit_render_calls(buffer, m_current_image_index.value(),
                                  {{0, 0}, m_swapchain.extent()}, clear_values);
      }

      buffer.end();
   }
}

void render_system::end_frame()
{
   if (m_images_in_flight.lookup(m_current_image_index.value()))
   {
      m_device.logical().waitForFences({m_images_in_flight.lookup(m_current_frame_index.value())},
                                       true, std::numeric_limits<std::uint64_t>::max());
   }
   m_images_in_flight.lookup(m_current_image_index.value()) =
      m_in_flight_fences.at(m_current_frame_index.value()).get();

   const std::array wait_semaphores{
      m_image_available_semaphores.at(m_current_frame_index.value()).get()};
   const std::array signal_semaphores{
      m_render_finished_semaphores.lookup(m_current_image_index.value()).get()};
   const std::array command_buffers{
      m_render_command_pools.at(m_current_frame_index.value()).primary_cmd_buffers().lookup(0)};
   const std::array<vk::PipelineStageFlags, 1> wait_stages{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

   m_device.logical().resetFences({m_in_flight_fences.at(m_current_frame_index.value()).get()});

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
      gfx_queue.submit(submit_infos, m_in_flight_fences.at(m_current_frame_index.value()).get());
   }
   catch (const vk::SystemError& err)
   {
      m_logger.error("[gfx] failed to submit graphics queue");

      std::terminate();
   }

   const std::array swapchains{vkn::value(m_swapchain)};

   const auto present_queue = m_device.get_queue(cacao::queue_flag_bits::present);
   if (present_queue.value.presentKHR({.waitSemaphoreCount = std::size(signal_semaphores),
                                       .pWaitSemaphores = std::data(signal_semaphores),
                                       .swapchainCount = std::size(swapchains),
                                       .pSwapchains = std::data(swapchains),
                                       .pImageIndices = &m_current_image_index.value()}) !=
       vk::Result::eSuccess)
   {
      m_logger.error("[gfx] failed to present present queue");

      std::terminate();
   }

   m_current_frame_index = (m_current_frame_index + 1) % max_frames_in_flight;
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
auto render_system::swapchain() const -> const vkn::swapchain&
{
   return m_swapchain;
}
auto render_system::swapchain() -> vkn::swapchain&
{
   return m_swapchain;
}

auto render_system::get_depth_attachment() const -> vk::ImageView
{
   return m_depth_image.view();
}

auto render_system::vertex_bindings() -> vertex_bindings_array
{
   return {
      {.binding = 0, .stride = sizeof(cacao::vertex), .inputRate = vk::VertexInputRate::eVertex}};
}
auto render_system::vertex_attributes() -> vertex_attributes_array
{
   return {{.location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(cacao::vertex, position)},
           {.location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(cacao::vertex, normal)},
           {.location = 2,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(cacao::vertex, colour)}};
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

auto render_system::create_vertex_buffer(const crl::dynamic_array<cacao::vertex>& vertices) const
   -> util::result<cacao::vertex_buffer>
{
   return cacao::vertex_buffer::make({.vertices = vertices,
                                      .device = m_device,
                                      .command_pool = m_render_command_pools[0],
                                      .logger = m_logger});
}
auto render_system::create_index_buffer(const crl::dynamic_array<std::uint32_t>& indices) const
   -> util::result<cacao::index_buffer>
{
   return cacao::index_buffer::make({.indices = indices,
                                     .device = m_device,
                                     .command_pool = m_render_command_pools[0],
                                     .logger = m_logger});
}

auto render_system::lookup_configuration() const -> const config&
{
   return m_configuration;
}
