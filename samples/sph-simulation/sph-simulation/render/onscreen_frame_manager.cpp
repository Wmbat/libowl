#include <sph-simulation/render/onscreen_frame_manager.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/generate_n.hpp>
#include <range/v3/view/iota.hpp>

namespace rv = ranges::views;

using namespace reglisse;

auto create_render_finished_semaphores(const cacao::device& device, mannele::u64 count)
   -> std::vector<vk::UniqueSemaphore>;
auto create_image_available_semaphores(const cacao::device& device)
   -> std::array<vk::UniqueSemaphore, max_frames_in_flight>;
auto create_in_flight_fences(const cacao::device& device)
   -> std::array<vk::UniqueFence, max_frames_in_flight>;

onscreen_frame_manager::onscreen_frame_manager(const onscreen_frame_manager_create_info& info) :
   m_logger(info.logger), mp_device(&info.device),
   m_swapchain(cacao::swapchain_create_info{
      .device = *mp_device,
      .surface = info.surface,
      .desired_formats = {{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}},
      .desired_present_modes = {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo},
      .desired_dimensions = info.window.dimension(),
      .graphics_queue_index = 0,
      .present_queue_index = 0,
      .image_usage_flags = vk::ImageUsageFlagBits::eColorAttachment,
      .composite_alpha_flags = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .should_clip = true,
      .old_swapchain = nullptr,
      .logger = m_logger}),
   m_render_finished_semaphores(
      create_render_finished_semaphores(*mp_device, std::size(m_swapchain.image_views()))),
   m_image_available_semaphores(create_image_available_semaphores(*mp_device)),
   m_in_flight_fences(create_in_flight_fences(*mp_device))
{
   m_images_in_flight.resize(std::size(m_swapchain.images()));
}

auto onscreen_frame_manager::begin_frame(std::span<cacao::command_pool> pools)
   -> reglisse::maybe<mannele::u32>
{
   const auto device = mp_device->logical();

   // NOLINTNEXTLINE
   const auto wait_res = device.waitForFences({m_in_flight_fences[m_current_frame_index].get()},
                                              true, std::numeric_limits<mannele::u64>::max());
   if (wait_res != vk::Result::eSuccess)
   {
      m_logger.error("failed to wait for fence at frame index {}: {}", m_current_frame_index,
                     vk::to_string(wait_res));

      return none;
   }

   const auto [image_res, image_index] = device.acquireNextImageKHR(
      m_swapchain.value(), std::numeric_limits<mannele::u64>::max(),
      m_image_available_semaphores[m_current_frame_index].get(), nullptr); // NOLINT

   if (image_res != vk::Result::eSuccess)
   {
      m_logger.error("failed to acquire next image: {}", vk::to_string(image_res));

      return none;
   }

   m_logger.debug(R"(swapchain image "{}" acquired)", image_index);

   // NOLINTNEXTLINE
   device.resetCommandPool(pools[m_current_frame_index].value(), {});

   m_current_image_index = image_index;

   return some(m_current_image_index);
}

// TODO: REWORK
void onscreen_frame_manager::end_frame(std::span<cacao::command_pool> pools)
{
   const auto device = mp_device->logical();

   if (m_images_in_flight.at(m_current_image_index))
   {
      [[maybe_unused]] auto _ =
         device.waitForFences({m_images_in_flight.at(m_current_frame_index)}, true,
                              std::numeric_limits<std::uint64_t>::max());
   }
   m_images_in_flight.at(m_current_image_index) =
      m_in_flight_fences.at(m_current_frame_index).get();

   const std::array wait_semaphores{m_image_available_semaphores.at(m_current_frame_index).get()};
   const std::array signal_semaphores{m_render_finished_semaphores.at(m_current_image_index).get()};
   const std::array command_buffers{pools[m_current_frame_index].primary_buffers()[0]};
   const std::array<vk::PipelineStageFlags, 1> wait_stages{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

   device.resetFences({m_in_flight_fences.at(m_current_frame_index).get()});

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
      const auto gfx_queue = mp_device->get_queue(cacao::queue_flag_bits::graphics).value;
      gfx_queue.submit(submit_infos, m_in_flight_fences.at(m_current_frame_index).get());
   }
   catch (const vk::SystemError& err)
   {
      m_logger.error("[gfx] failed to submit graphics queue");

      std::terminate();
   }

   const std::array swapchains{m_swapchain.value()};

   const auto present_queue = mp_device->get_queue(cacao::queue_flag_bits::present);
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

   m_current_frame_index = (m_current_frame_index + 1) % max_frames_in_flight;
}

auto create_render_finished_semaphores(const cacao::device& device, mannele::u64 count)
   -> std::vector<vk::UniqueSemaphore>
{
   const auto create = [&] {
      return device.logical().createSemaphoreUnique({});
   };

   return rv::generate_n(create, count) | ranges::to_vector;
}

auto create_image_available_semaphores(const cacao::device& device)
   -> std::array<vk::UniqueSemaphore, max_frames_in_flight>
{
   std::array<vk::UniqueSemaphore, max_frames_in_flight> semaphores;

   for (auto& semaphore : semaphores)
   {
      semaphore = device.logical().createSemaphoreUnique({});
   }

   return semaphores;
}

auto create_in_flight_fences(const cacao::device& device)
   -> std::array<vk::UniqueFence, max_frames_in_flight>
{
   std::array<vk::UniqueFence, max_frames_in_flight> fences;
   for (auto& fence : fences)
   {
      fence = device.logical().createFenceUnique({.flags = vk::FenceCreateFlagBits::eSignaled});
   }

   return fences;
}
