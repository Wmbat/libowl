/**
 * @file libowl/gfx/render_target.cpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief
 * @copyright Copyright (C) 2022 wmbat
 */

#include <libowl/gfx/render_target.hpp>

#include <libowl/types.hpp>

#include <ranges>

namespace stdr = std::ranges;

namespace
{
   static constexpr std::array desired_formats = {
      vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear),
      vk::SurfaceFormatKHR(vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear)};

   static constexpr std::array desired_present_modes = {vk::PresentModeKHR::eMailbox,
                                                        vk::PresentModeKHR::eFifo};

   auto find_best_surface_format(std::span<const vk::SurfaceFormatKHR> available_formats)
      -> vk::SurfaceFormatKHR
   {
      auto const it = stdr::find_first_of(available_formats, desired_formats);
      return it != stdr::end(available_formats) ? *it : available_formats[0];
   }

   auto find_best_present_mode(std::span<const vk::PresentModeKHR> available_modes)
      -> vk::PresentModeKHR
   {
      auto const it = stdr::find_first_of(available_modes, desired_present_modes);
      return it != stdr::end(available_modes) ? *it : vk::PresentModeKHR::eFifo;
   }

   auto find_best_extent(const vk::SurfaceCapabilitiesKHR& capabilities,
                         owl::monitor_dimensions const& dimensions) -> vk::Extent2D
   {
      if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
      {
         return capabilities.currentExtent;
      }

      return vk::Extent2D()
         .setWidth(std::clamp(owl::u32(dimensions.width), capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width))
         .setHeight(std::clamp(owl::u32(dimensions.height), capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height));
   }
} // namespace

namespace owl::inline v0
{
   render_target::render_target(vk::UniqueSurfaceKHR&& surface,
                                monitor_dimensions const& dimensions, spdlog::logger& logger) :
      m_dimensions(dimensions),
      m_surface(std::move(surface)), mp_logger(&logger)
   {}

   [[nodiscard]] auto render_target::surface() const noexcept -> vk::SurfaceKHR
   {
      // NOLINTNEXTLINE
      assert(static_cast<VkSurfaceKHR>(m_surface.get()) != VK_NULL_HANDLE);

      return m_surface.get();
   }

   void render_target::set_device(gfx::device&& device)
   {
      if (m_device != device)
      {
         if (m_device)
         {
            m_status = target_status::device_lost;

            // destroy stuff
         }

         m_device = std::move(device);
         m_swapchain = create_swapchain();
         // create new stuff
      }
   }
   void render_target::update_dimensions(monitor_dimensions const& dimensions)
   {
      m_dimensions = dimensions;
   }

   auto render_target::create_swapchain() -> vk::UniqueSwapchainKHR
   {
      // NOLINTNEXTLINE
      assert(m_device.has_value());

      vk::PhysicalDevice physical_device = m_device->physical();
      vk::SurfaceKHR surface = m_surface.get();

      auto const capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
      auto const available_formats = physical_device.getSurfaceFormatsKHR(surface);
      auto const available_present_modes = physical_device.getSurfacePresentModesKHR(surface);

      auto const surface_format = find_best_surface_format(available_formats);
      auto const present_mode = find_best_present_mode(available_present_modes);
      auto const extent = find_best_extent(capabilities, m_dimensions);

      std::uint32_t image_count = capabilities.minImageCount + 1;
      if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
      {
         image_count = capabilities.maxImageCount;
      }

      std::span device_queues = m_device->logical().queues();

      // TODO(wmbat): Check that queues are actually valid
      auto const graphics_queue =
         stdr::find(device_queues, vk::QueueFlagBits::eGraphics, &ash::queue::type);
      auto const present_queue =
         stdr::find(device_queues, vk::QueueFlagBits::eTransfer, &ash::queue::type);

      std::array const queues = {graphics_queue->family_index, present_queue->family_index};
      auto const sharing_mode = graphics_queue == present_queue ? vk::SharingMode::eExclusive
                                                                : vk::SharingMode::eConcurrent;

      auto swapchain =
         static_cast<vk::Device>(m_device->logical())
            .createSwapchainKHRUnique(vk::SwapchainCreateInfoKHR()
                                         .setSurface(m_surface.get())
                                         .setMinImageCount(image_count)
                                         .setImageFormat(surface_format.format)
                                         .setImageColorSpace(surface_format.colorSpace)
                                         .setImageExtent(extent)
                                         .setImageArrayLayers(1u)
                                         .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                                         .setImageSharingMode(sharing_mode)
                                         .setQueueFamilyIndices(queues)
                                         .setPreTransform(capabilities.currentTransform)
                                         .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                                         .setPresentMode(present_mode)
                                         .setClipped(true)
                                         .setOldSwapchain(nullptr));

      if (mp_logger)
      {
         mp_logger->debug("Swapchain created with {} images", image_count);
      }

      return swapchain;
   }
} // namespace owl::inline v0
