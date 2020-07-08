#include <epona_core/graphics/vkn/swapchain.hpp>

#include <algorithm>
#include <array>
#include <limits>

namespace core::gfx::vkn
{
   namespace detail
   {
      struct surface_support
      {
         vk::SurfaceCapabilitiesKHR capabilities;
         util::dynamic_array<vk::SurfaceFormatKHR> formats;
         util::dynamic_array<vk::PresentModeKHR> present_modes;

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

      auto to_string(swapchain::error err) -> std::string
      {
         using error = swapchain::error;

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

      struct swapchain_error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override { return "vk_instance"; }
         [[nodiscard]] auto message(int err) const -> std::string override
         {
            return to_string(static_cast<swapchain::error>(err));
         }
      };

      static const swapchain_error_category inst_err_cat{};

      auto make_error_code(swapchain::error err) -> std::error_code
      {
         return {static_cast<int>(err), inst_err_cat};
      }

      auto query_surface_support(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
         -> vkn::result<surface_support>
      {
         if (surface)
         {
            // clang-format off
            return util::monad::to_error(error{
               .type = make_error_code(surface_support::error::surface_handle_not_provided),
               .result = {}
            });
            // clang-format on
         }

         const auto capabilities_res = util::monad::try_wrap<vk::SystemError>([=] {
            return physical_device.getSurfaceCapabilitiesKHR(surface);
         });

         if (capabilities_res.is_left())
         {
            // clang-format off
            return util::monad::to_error(error{
               .type = make_error_code(surface_support::error::failed_to_get_surface_capabilities),
               .result = static_cast<vk::Result>(capabilities_res.left()->code().value())
            });
            // clang-format on
         }

         const auto format_res = util::monad::try_wrap<std::system_error>([=] {
            return physical_device.getSurfaceFormatsKHR(surface);
         });

         if (format_res.is_left())
         {
            // clang-format off
            return util::monad::to_error(error{
               .type = make_error_code(surface_support::error::failed_to_enumerate_formats),
               .result = static_cast<vk::Result>(format_res.left()->code().value())
            });
            // clang-format on
         }

         const auto present_mode_res = util::monad::try_wrap<std::system_error>([=] {
            return physical_device.getSurfacePresentModesKHR(surface);
         });

         if (present_mode_res.is_left())
         {
            // clang-format off
            return util::monad::to_error(error{
               .type = make_error_code(surface_support::error::failed_to_enumerate_formats),
               .result = static_cast<vk::Result>(present_mode_res.left()->code().value())
            });
            // clang-format on
         }

         // clang-format off
         return util::monad::to_value(surface_support{
            .capabilities = capabilities_res.right().value(), 
            .formats = format_res.right().value(), 
            .present_modes = present_mode_res.right().value()
         });
         // clang-format on
      }

      auto find_surface_format(const util::range_over<vk::SurfaceFormatKHR> auto& available_formats,
         const util::range_over<vk::SurfaceFormatKHR> auto& desired_formats) -> vk::SurfaceFormatKHR
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

      auto find_present_mode(
         const util::range_over<vk::PresentModeKHR> auto& available_present_modes,
         const util::range_over<vk::PresentModeKHR> auto& desired_present_modes)
         -> vk::PresentModeKHR
      {
         for (const vk::PresentModeKHR& desired : desired_present_modes)
         {
            for (const vk::PresentModeKHR& available : available_present_modes)
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
         else
         {
            // clang-format off
            return vk::Extent2D{}
               .setWidth(std::clamp(desired_width, capabilities.minImageExtent.width,
                  capabilities.maxImageExtent.width))
               .setHeight(std::clamp(desired_height, capabilities.minImageExtent.height, 
                  capabilities.maxImageExtent.height));
            // clang-format on
         }
      }
   } // namespace detail

   swapchain::swapchain(swapchain&& rhs) noexcept { *this = std::move(rhs); }
   swapchain::swapchain(const create_info& info) noexcept :
      m_device(info.device), m_swapchain(info.swapchain), m_format(info.format),
      m_extent(info.extent)
   {}
   swapchain::~swapchain()
   {
      if (m_device && m_swapchain)
      {
         m_device.destroySwapchainKHR(m_swapchain);
      }
   }

   auto swapchain::operator=(swapchain&& rhs) noexcept -> swapchain&
   {
      if (this != &rhs)
      {
         m_swapchain = rhs.m_swapchain;
         rhs.m_swapchain = nullptr;

         m_device = rhs.m_device;
         rhs.m_device = nullptr;

         m_format = rhs.m_format;
         rhs.m_format = vk::Format{};

         m_extent = rhs.m_extent;
         rhs.m_extent = vk::Extent2D{};
      }

      return *this;
   }

   auto swapchain::value() const -> const vk::SwapchainKHR& { return m_swapchain; }

   using builder = swapchain::builder;

   builder::builder(const device& device)
   {
      m_info.device = device.value();
      m_info.physical_device = device.physical().value();
      m_info.surface = device.physical().surface();

      if (const auto maybe = device.get_queue_index(queue::type::graphics))
      {
         m_info.graphics_queue_index = maybe.value().value();
      }

      if (const auto maybe = device.get_queue_index(queue::type::present))
      {
         m_info.present_queue_index = maybe.value().value();
      }
   }

   auto builder::build() -> vkn::result<swapchain>
   {
      using err_t = vkn::error;

      if (!m_info.surface)
      {
         // clang-format off
         return util::monad::to_error(err_t{
            .type = detail::make_error_code(swapchain::error::surface_handle_not_provided),
            .result = {}
         });
         // clang-format on
      }

      const auto surface_support_res =
         detail::query_surface_support(m_info.physical_device, m_info.surface);
      if (!surface_support_res)
      {
         // clang-format off
         return util::monad::to_error(err_t{
            .type = detail::make_error_code(swapchain::error::failed_to_query_surface_support_details),
            .result = surface_support_res.error().value().result
         });
         // clang-format on
      }

      const auto surface_support = *surface_support_res.value();

      util::tiny_dynamic_array<vk::SurfaceFormatKHR, 2> desired_formats = m_info.desired_formats;
      if (desired_formats.empty())
      {
         desired_formats = add_desired_formats();
      }

      util::tiny_dynamic_array<vk::PresentModeKHR, 2> desired_present_modes =
         m_info.desired_present_modes;
      if (desired_present_modes.empty())
      {
         desired_present_modes = add_desired_present_modes();
      }

      const vk::SurfaceFormatKHR surface_format =
         detail::find_surface_format(surface_support.formats, desired_formats);

      const vk::PresentModeKHR present_mode =
         detail::find_present_mode(surface_support.present_modes, desired_present_modes);

      const vk::Extent2D extent = detail::find_extent(
         surface_support.capabilities, m_info.desired_width, m_info.desired_height);

      uint32_t image_count = surface_support.capabilities.minImageCount + 1;
      if (surface_support.capabilities.maxImageCount > 0 &&
         image_count > surface_support.capabilities.maxImageCount)
      {
         image_count = surface_support.capabilities.maxImageCount;
      }

      const auto queue_family_indices =
         std::array<uint32_t, 2>{m_info.graphics_queue_index, m_info.present_queue_index};

      const bool same = m_info.graphics_queue_index == m_info.present_queue_index;

      // clang-format off
      const auto create_info = vk::SwapchainCreateInfoKHR{}
         .setFlags({})
         .setPNext(nullptr)
         .setSurface(m_info.surface)
         .setMinImageCount(image_count)
         .setImageFormat(surface_format.format)
         .setImageColorSpace(surface_format.colorSpace)
         .setImageExtent(extent)
         .setImageUsage(m_info.image_usage_flags)
         .setImageSharingMode(same ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent)
         .setQueueFamilyIndexCount(same ? 0 : 2)
         .setPQueueFamilyIndices(same ? nullptr : queue_family_indices.data())
         .setPreTransform(surface_support.capabilities.currentTransform)
         .setCompositeAlpha(m_info.composite_alpha_flags)
         .setPresentMode(present_mode)
         .setClipped(m_info.clipped);
      // clang-format on  
  
      const auto creation_res = util::monad::try_wrap<vk::SystemError>([&]{
         return m_info.device.createSwapchainKHR(create_info);
      });

      if(creation_res.is_left())
      {
         // clang-format off
         return util::monad::to_error(err_t{
            .type = detail::make_error_code(error::failed_to_create_swapchain), 
            .result = static_cast<vk::Result>(creation_res.left()->code().value())
         });
         // clang-format on
      }

      // clang-format off
      const auto swap_create_info = swapchain::create_info{
         .device = m_info.device,
         .swapchain = creation_res.right().value(),
         .format = surface_format.format,
         .extent = extent
      };
      // clang-format on

      return util::monad::to_value(swapchain{swap_create_info});
   }

   auto builder::set_old_swapchain(const swapchain& swap) noexcept -> builder&
   {
      m_info.old_swapchain = swap.value();
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

   auto builder::add_desired_formats() const -> util::tiny_dynamic_array<vk::SurfaceFormatKHR, 2>
   {
      // clang-format off
      return util::tiny_dynamic_array<vk::SurfaceFormatKHR, 2>{{
         vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
         vk::SurfaceFormatKHR{vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}
      }};
      // clang-format on
   }
   auto builder::add_desired_present_modes() const
      -> util::tiny_dynamic_array<vk::PresentModeKHR, 2>
   {
      // clang-format off
      return util::tiny_dynamic_array<vk::PresentModeKHR, 2>{{
         vk::PresentModeKHR::eMailbox,
         vk::PresentModeKHR::eFifo
      }};
      // clang-format on
   }

} // namespace core::gfx::vkn
