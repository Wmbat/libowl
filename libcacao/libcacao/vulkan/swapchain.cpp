#include <libcacao/vulkan/swapchain.hpp>

#include <libreglisse/operations/and_then.hpp>
#include <libreglisse/try.hpp>

#include <algorithm>
#include <array>
#include <limits>

using namespace reglisse;

namespace vkn
{
   namespace detail
   {
      struct surface_support
      {
         vk::SurfaceCapabilitiesKHR capabilities;
         std::vector<vk::SurfaceFormatKHR> formats;
         std::vector<vk::PresentModeKHR> present_modes;

         enum class error
         {
            surface_handle_not_provided,
            failed_to_get_surface_capabilities,
            failed_to_enumerate_formats,
            failed_to_enumerate_present_modes
         };
      };

      auto to_string(surface_support::error err) -> std::string
      {
         using error = surface_support::error;

         switch (err)
         {
            case error::surface_handle_not_provided:
               return "surface_handle_not_provided";
            case error::failed_to_get_surface_capabilities:
               return "failed_to_get_surface_capabilities";
            case error::failed_to_enumerate_formats:
               return "failed_to_enumerate_formats";
            case error::failed_to_enumerate_present_modes:
               return "failed_to_enumerate_present_modes";
            default:
               return "UNKNOWN";
         }
      }

      struct surface_support_error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override { return "vk_instance"; }
         [[nodiscard]] auto message(int err) const -> std::string override
         {
            return to_string(static_cast<surface_support::error>(err));
         }
      };

      static const surface_support_error_category surface_support_err_cat{};

      auto make_error_code(surface_support::error err) -> std::error_code
      {
         return {static_cast<int>(err), surface_support_err_cat};
      }

      auto query_surface_support(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
         -> util::result<surface_support>
      {
         if (!surface)
         {
            return err(
               util::error_t{make_error_code(surface_support::error::surface_handle_not_provided)});
         }

         const auto capabilities_res = try_wrap<vk::SystemError>([=] {
            return physical_device.getSurfaceCapabilitiesKHR(surface);
         });

         if (!capabilities_res)
         {
            return err(util::error_t{
               make_error_code(surface_support::error::failed_to_get_surface_capabilities)});
         }

         const auto format_res = try_wrap<std::system_error>([=] {
            return physical_device.getSurfaceFormatsKHR(surface);
         });

         if (!format_res)
         {
            return err(
               util::error_t{make_error_code(surface_support::error::failed_to_enumerate_formats)});
         }

         const auto present_mode_res = try_wrap<std::system_error>([=] {
            return physical_device.getSurfacePresentModesKHR(surface);
         });

         if (!present_mode_res)
         {
            // clang-format off
            return err(util::error_t{
               make_error_code(surface_support::error::failed_to_enumerate_formats)
            });
            // clang-format on
         }

         const auto formats = format_res.value().value();
         const auto present_mode = present_mode_res.value().value();

         return surface_support
         {
            .capabilities = capabilities_res.value().value(), .formats = formats,
            .present_modes = present_mode;
         }
      }

      auto find_surface_format(std::span<const vk::SurfaceFormatKHR> available_formats,
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

      auto find_present_mode(std::span<const vk::PresentModeKHR> available_present_modes,
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

      auto find_extent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t desired_width,
                       uint32_t desired_height) -> vk::Extent2D
      {
         if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
         {
            return capabilities.currentExtent;
         }

         return vk::Extent2D{}
            .setWidth(std::clamp(desired_width, capabilities.minImageExtent.width,
                                 capabilities.maxImageExtent.width))
            .setHeight(std::clamp(desired_height, capabilities.minImageExtent.height,
                                  capabilities.maxImageExtent.height));
      }

      auto to_string(swapchain::error_type err) -> std::string
      {
         using error = swapchain::error_type;

         switch (err)
         {
            case error::surface_handle_not_provided:
               return "surface_handle_not_provided";
            case error::failed_to_query_surface_support_details:
               return "failed_to_query_surface_support_details";
            case error::failed_to_get_swapchain_images:
               return "failed_to_get_swapchain_images";
            case error::failed_to_create_swapchain_image_views:
               return "failed_to_create_swapchain_image_views";
            case error::failed_to_create_swapchain:
               return "failed_to_create_swapchain";
            default:
               return "UNKNOWN";
         }
      };
   } // namespace detail

   auto swapchain::error_category::name() const noexcept -> const char* { return "vkn_swapchain"; }
   auto swapchain::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<swapchain::error_type>(err));
   }

   swapchain::swapchain(create_info&& info) noexcept :
      m_swapchain{std::move(info.swapchain)},
      m_format(info.format),
      m_extent(info.extent),
      m_images{std::move(info.images)},
      m_image_views{std::move(info.image_views)}
   {}

   auto swapchain::operator->() noexcept -> pointer { return &m_swapchain.get(); }
   auto swapchain::operator->() const noexcept -> const_pointer { return &m_swapchain.get(); }

   auto swapchain::operator*() const noexcept -> value_type { return value(); }

   swapchain::operator bool() const noexcept { return m_swapchain.get(); }

   auto swapchain::value() const noexcept -> vk::SwapchainKHR { return m_swapchain.get(); }
   auto swapchain::format() const noexcept -> vk::Format { return m_format; }
   auto swapchain::extent() const noexcept -> const vk::Extent2D& { return m_extent; }
   auto swapchain::image_views() const noexcept -> const std::vector<vk::UniqueImageView>&
   {
      return m_image_views;
   }

   using builder = swapchain::builder;

   builder::builder(const cacao::device& device, vk::SurfaceKHR surface,
                    util::logger_wrapper logger) :
      m_logger{logger}
   {
      m_info.device = device.logical();
      m_info.physical_device = device.physical();
      m_info.surface = surface;
      m_info.graphics_queue_index = device.get_queue_index(cacao::queue_flag_bits::graphics);
      m_info.present_queue_index = device.get_queue_index(cacao::queue_flag_bits::present);
   }

   auto builder::build() -> swapchain
   {
      using err_t = swapchain::error_type;
      if (!m_info.surface)
      {
         // return make_error_res(err_t::surface_handle_not_provided);
      }

      const auto surface_support_res =
         detail::query_surface_support(m_info.physical_device, m_info.surface);
      if (!surface_support_res)
      {
         // return err(make_error(err_t::failed_to_query_surface_support_details));
      }

      const auto surface_support = surface_support_res.borrow();

      std::vector<vk::SurfaceFormatKHR> desired_formats = m_info.desired_formats;
      if (desired_formats.empty())
      {
         desired_formats = add_desired_formats();
      }

      std::vector<vk::PresentModeKHR> desired_present_modes = m_info.desired_present_modes;
      if (desired_present_modes.empty())
      {
         desired_present_modes = add_desired_present_modes();
      }

      const vk::SurfaceFormatKHR surface_format =
         detail::find_surface_format(surface_support.formats, desired_formats);

      const vk::PresentModeKHR present_mode =
         detail::find_present_mode(surface_support.present_modes, desired_present_modes);

      const vk::Extent2D extent = detail::find_extent(surface_support.capabilities,
                                                      m_info.desired_width, m_info.desired_height);

      uint32_t image_count = surface_support.capabilities.minImageCount + 1;
      if (surface_support.capabilities.maxImageCount > 0 &&
          image_count > surface_support.capabilities.maxImageCount)
      {
         image_count = surface_support.capabilities.maxImageCount;
      }

      const auto queue_family_indices =
         std::array<uint32_t, 2>{m_info.graphics_queue_index, m_info.present_queue_index};

      const bool same = m_info.graphics_queue_index == m_info.present_queue_index;

      auto swapchain_create_info =
         vk::SwapchainCreateInfoKHR{}
            .setSurface(m_info.surface)
            .setMinImageCount(image_count)
            .setImageFormat(surface_format.format)
            .setImageColorSpace(surface_format.colorSpace)
            .setImageExtent(extent)
            .setImageArrayLayers(1u)
            .setImageUsage(m_info.image_usage_flags)
            .setImageSharingMode(same ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(same ? 0u : 2u)
            .setPQueueFamilyIndices(same ? nullptr : queue_family_indices.data())
            .setPreTransform(surface_support.capabilities.currentTransform)
            .setCompositeAlpha(m_info.composite_alpha_flags)
            .setPresentMode(present_mode)
            .setClipped(m_info.clipped)
            .setOldSwapchain(m_info.old_swapchain);

      return create_swapchain(std::move(swapchain_create_info)) |
         and_then([&](vk::UniqueSwapchainKHR&& handle) {
                m_logger.info("[vkn] swapchain created.");
                m_logger.info("[vkn] swapchain image count: {0}", image_count);

                return create_images(handle.get()).and_then([&](auto&& images) {
                   return create_image_views(images, surface_format).map([&](auto&& views) {
                      return swapchain{{.swapchain = std::move(handle),
                                        .format = surface_format.format,
                                        .extent = extent,
                                        .images = std::move(images),
                                        .image_views = std::move(views)}};
                   });
                });
             });
   }

   auto builder::set_old_swapchain(const swapchain& swap) noexcept -> builder&
   {
      m_info.old_swapchain = vkn::value(swap);
      return *this;
   }

   auto builder::set_desired_extent(uint32_t width, uint32_t height) noexcept -> builder&
   {
      m_info.desired_width = width;
      m_info.desired_height = height;

      return *this;
   }

   auto builder::set_desired_format(const vk::SurfaceFormatKHR& format) -> builder&
   {
      m_info.desired_formats.insert(m_info.desired_formats.cbegin(), format);
      return *this;
   }
   auto builder::add_fallback_format(const vk::SurfaceFormatKHR& format) -> builder&
   {
      m_info.desired_formats.push_back(format);
      return *this;
   }

   auto builder::use_default_format_selection() -> builder&
   {
      m_info.desired_formats.clear();
      m_info.desired_formats = add_desired_formats();

      return *this;
   }

   auto builder::set_desired_present_mode(vk::PresentModeKHR present_mode) -> builder&
   {
      m_info.desired_present_modes.insert(m_info.desired_present_modes.cbegin(), present_mode);
      return *this;
   }
   auto builder::add_fallback_present_mode(vk::PresentModeKHR present_mode) -> builder&
   {
      m_info.desired_present_modes.push_back(present_mode);
      return *this;
   }

   auto builder::use_default_present_mode_selection() -> builder&
   {
      m_info.desired_present_modes.clear();
      m_info.desired_present_modes = add_desired_present_modes();

      return *this;
   }

   auto builder::set_image_usage_flags(const vk::ImageUsageFlags& usage_flags) noexcept -> builder&
   {
      m_info.image_usage_flags = usage_flags;
      return *this;
   }
   auto builder::add_image_usage_flags(const vk::ImageUsageFlags& usage_flags) noexcept -> builder&
   {
      m_info.image_usage_flags |= usage_flags;
      return *this;
   }
   auto builder::use_default_image_usage_flags() noexcept -> builder&
   {
      m_info.image_usage_flags = vk::ImageUsageFlagBits::eColorAttachment;
      return *this;
   }

   auto builder::set_composite_alpha_flags(vk::CompositeAlphaFlagBitsKHR alpha_flags) noexcept
      -> builder&
   {
      m_info.composite_alpha_flags = alpha_flags;
      return *this;
   }

   auto builder::set_clipped(bool clipped) noexcept -> builder&
   {
      m_info.clipped = clipped ? VK_TRUE : VK_FALSE;
      return *this;
   }

   auto builder::add_desired_formats() const -> std::vector<vk::SurfaceFormatKHR>
   {
      return std::vector<vk::SurfaceFormatKHR>{
         {vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
          vk::SurfaceFormatKHR{vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}}};
   }
   auto builder::add_desired_present_modes() const -> std::vector<vk::PresentModeKHR>
   {
      return std::vector<vk::PresentModeKHR>{
         {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo}};
   }

   auto builder::create_swapchain(vk::SwapchainCreateInfoKHR&& info) const
      -> util::result<vk::UniqueSwapchainKHR>
   {
      return try_wrap<vk::SystemError>([&] {
                return m_info.device.createSwapchainKHRUnique(info);
             })
         .map_error([]([[maybe_unused]] vk::SystemError&& err) {
            return make_error(error_type::failed_to_create_swapchain);
         });
   }
   auto builder::create_images(vk::SwapchainKHR swapchain) const
      -> util::result<std::vector<vk::Image>>
   {
      return try_wrap<std::system_error>([&] {
                return m_info.device.getSwapchainImagesKHR(swapchain);
             })
         .map_error([]([[maybe_unused]] const std::system_error& err) {
            return make_error(error_type::failed_to_get_swapchain_images);
         })
         .map([](const auto& images) {
            return std::vector<vk::Image>{images.begin(), images.end()};
         });
   }

   auto builder::create_image_views(const std::vector<vk::Image>& images,
                                    vk::SurfaceFormatKHR format) const
      -> util::result<std::vector<vk::UniqueImageView>>
   {
      std::vector<vk::UniqueImageView> views{};

      if (!try_wrap<std::bad_alloc>([&] {
             views.reserve(images.size());
             return 0;
          }))
      {
         return err(make_error(swapchain::error_type::failed_to_create_swapchain_image_views));
      }

      for (const auto& image : images)
      {
         // clang-format off
         const auto create_info = vk::ImageViewCreateInfo{}
            .setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(format.format)
            .setComponents(vk::ComponentMapping{}
               .setR(vk::ComponentSwizzle::eIdentity)
               .setG(vk::ComponentSwizzle::eIdentity)
               .setB(vk::ComponentSwizzle::eIdentity)
               .setA(vk::ComponentSwizzle::eIdentity))
            .setSubresourceRange(vk::ImageSubresourceRange{}
               .setAspectMask(vk::ImageAspectFlagBits::eColor)
               .setBaseMipLevel(0U)
               .setLevelCount(1U)
               .setBaseArrayLayer(0U)
               .setLayerCount(1U));
         // clang-format on

         const auto view_res = monad::try_wrap<vk::SystemError>([&] {
                                  return m_info.device.createImageViewUnique(create_info);
                               }).map([&](vk::UniqueImageView&& handle) {
            views.append(std::move(handle));
            return 0;
         });

         if (!view_res)
         {
            return monad::err(make_error(error_type::failed_to_create_swapchain_image_views));
         }
      }

      return views;
   }
} // namespace vkn
