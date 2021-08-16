#include <libcacao/swapchain.hpp>

#include <libmannele/dimension.hpp>

#include <libreglisse/operations/transform_err.hpp>
#include <libreglisse/try.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

using namespace reglisse;
using namespace mannele;

namespace cacao
{
   static constexpr auto DEFAULT_FORMATS = std::array<vk::SurfaceFormatKHR, 2>(
      {vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
       vk::SurfaceFormatKHR{vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}});

   static constexpr auto DEFAULT_PRESENT_MODES =
      std::array<vk::PresentModeKHR, 2>({vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo});

   auto find_best_surface_format(std::span<const vk::SurfaceFormatKHR> available_formats,
                                 std::span<const vk::SurfaceFormatKHR> desired_formats)
      -> vk::SurfaceFormatKHR
   {
      for (const vk::SurfaceFormatKHR& desired : desired_formats)
      {
         for (const vk::SurfaceFormatKHR& available : available_formats)
         {
            if (desired.format == available.format && desired.colorSpace == available.colorSpace)
            {
               return desired;
            }
         }
      }

      return available_formats[0];
   }

   auto find_best_present_mode(std::span<const vk::PresentModeKHR> available_present_modes,
                               std::span<const vk::PresentModeKHR> desired_present_modes)
      -> vk::PresentModeKHR
   {
      for (const auto desired : desired_present_modes)
      {
         for (const auto available : available_present_modes)
         {
            if (desired == available)
            {
               return desired;
            }
         }
      }

      return vk::PresentModeKHR::eFifo;
   }

   auto find_best_extent(const vk::SurfaceCapabilitiesKHR& capabilities,
                         mannele::dimension_u32 desired_dimensions) -> vk::Extent2D
   {
      if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
      {
         return capabilities.currentExtent;
      }

      return vk::Extent2D{
         .width = std::clamp(desired_dimensions.width, capabilities.minImageExtent.width,
                             capabilities.maxImageExtent.width),
         .height = std::clamp(desired_dimensions.height, capabilities.minImageExtent.height,
                              capabilities.maxImageExtent.height)};
   }

   swapchain::swapchain(const swapchain_create_info& info) : m_logger(info.logger)
   {
      const auto device = info.device.logical();
      const auto surface_support_res = query_surface_support(info.device, info.surface);

      if (!surface_support_res)
      {
         throw runtime_error(to_error_condition(surface_support_res.borrow_err()));
      }

      const auto& surface_support = surface_support_res.borrow();

      const std::span desired_formats = std::empty(info.desired_formats)
         ? std::span(DEFAULT_FORMATS)
         : std::span(info.desired_formats);

      const std::span desired_present_modes = std::empty(info.desired_present_modes)
         ? std::span(DEFAULT_PRESENT_MODES)
         : std::span(info.desired_present_modes);

      const vk::SurfaceFormatKHR format =
         find_best_surface_format(surface_support.formats, desired_formats);
      const vk::PresentModeKHR present_mode =
         find_best_present_mode(surface_support.present_modes, desired_present_modes);
      const vk::Extent2D extent =
         find_best_extent(surface_support.capabilities, info.desired_dimensions);

      std::uint32_t image_count = surface_support.capabilities.minImageCount + 1;
      if (surface_support.capabilities.maxImageCount > 0 &&
          image_count > surface_support.capabilities.maxImageCount)
      {
         image_count = surface_support.capabilities.maxImageCount;
      }

      const std::array queue_family_indices = {info.graphics_queue_index, info.present_queue_index};
      const bool same_family = info.graphics_queue_index == info.present_queue_index;

      const auto create_info = vk::SwapchainCreateInfoKHR{
         .surface = info.surface.value(),
         .minImageCount = image_count,
         .imageFormat = format.format,
         .imageColorSpace = format.colorSpace,
         .imageExtent = extent,
         .imageArrayLayers = 1u,
         .imageUsage = info.image_usage_flags,
         .imageSharingMode =
            same_family ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
         .queueFamilyIndexCount = same_family ? 0u : 2u,
         .pQueueFamilyIndices = same_family ? nullptr : queue_family_indices.data(),
         .preTransform = surface_support.capabilities.currentTransform,
         .compositeAlpha = info.composite_alpha_flags,
         .presentMode = present_mode,
         .clipped = info.should_clip,
         .oldSwapchain = info.old_swapchain ? info.old_swapchain->value() : VK_NULL_HANDLE};

      m_swapchain = device.createSwapchainKHRUnique(create_info);
      m_images = device.getSwapchainImagesKHR(m_swapchain.get());
      m_extent = extent;
      m_format = format.format;

      for (const auto& image : m_images)
      {
         m_image_views.push_back(device.createImageViewUnique(
            {.image = image,
             .viewType = vk::ImageViewType::e2D,
             .format = m_format,
             .components = {.r = vk::ComponentSwizzle::eIdentity,
                            .g = vk::ComponentSwizzle::eIdentity,
                            .b = vk::ComponentSwizzle::eIdentity,
                            .a = vk::ComponentSwizzle::eIdentity},
             .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                  .baseMipLevel = 0u,
                                  .levelCount = 1u,
                                  .baseArrayLayer = 0u,
                                  .layerCount = 1u}}));
      }

      m_logger.info("Swapchain created with {} {}x{} images", std::size(m_images),
                    info.desired_dimensions.width, info.desired_dimensions.height);
   }

   auto swapchain::value() const noexcept -> vk::SwapchainKHR { return m_swapchain.get(); }
   auto swapchain::format() const noexcept -> vk::Format { return m_format; }
   auto swapchain::extent() const noexcept -> const vk::Extent2D& { return m_extent; }
   auto swapchain::images() const noexcept -> std::span<const vk::Image> { return m_images; }
   auto swapchain::image_views() const noexcept -> std::vector<vk::ImageView>
   {
      return m_image_views | ranges::views::transform([](const vk::UniqueImageView& view) {
                return view.get();
             }) |
         ranges::to_vector;
   }

   auto get_surface_capabilities(const device& device, const surface& surface)
   {
      return device.physical().getSurfaceCapabilitiesKHR(surface.value());
   }

   auto get_surface_format(const device& device, const surface& surface)
   {
      return device.physical().getSurfaceFormatsKHR(surface.value());
   }

   auto get_surface_present_mode(const device& device, const surface& surface)
   {
      return device.physical().getSurfacePresentModesKHR(surface.value());
   }

   auto query_surface_support(const device& device, const surface& surface)
      -> reglisse::result<surface_support, surface_support_error>
   {
      const auto surface_capabilities_res =
         try_wrap<vk::SystemError>(get_surface_capabilities, device, surface);
      const auto surface_format_res =
         try_wrap<vk::SystemError>(get_surface_format, device, surface);
      const auto surface_present_mode_res =
         try_wrap<vk::SystemError>(get_surface_present_mode, device, surface);

      if (!surface_capabilities_res)
      {
         return err(surface_support_error::failed_to_get_surface_capabilities);
      }

      if (!surface_format_res)
      {
         return err(surface_support_error::failed_to_enumerate_formats);
      }

      if (!surface_present_mode_res)
      {
         return err(surface_support_error::failed_to_enumerate_present_modes);
      }

      return ok(surface_support{.capabilities = surface_capabilities_res.borrow(),
                                .formats = surface_format_res.borrow(),
                                .present_modes = surface_present_mode_res.borrow()});
   }
} // namespace cacao
