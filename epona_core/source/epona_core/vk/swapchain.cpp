#include "epona_core/vk/swapchain.hpp"
#include "epona_core/details/monad/either.hpp"

#include <functional>
#include <system_error>

namespace core::vk
{
   namespace detail
   {
      enum class surface_support_error
      {
         physical_device_handle_null,
         surface_handle_null,
         failed_get_surface_capabilities,
         failed_enumerate_surface_formats,
         failed_enumerate_surface_present_modes
      };

      struct surface_support_error_category : std::error_category
      {
         const char* name() const noexcept override { return "vk_surface_support"; }
         std::string message(int err) const override
         {
            switch (static_cast<surface_support_error>(err))
            {
               case surface_support_error::physical_device_handle_null:
                  return "physical_device_handle_null";
               case surface_support_error::surface_handle_null:
                  return "surface_handle_null";
               case surface_support_error::failed_get_surface_capabilities:
                  return "failed_get_surface_capabilities";
               case surface_support_error::failed_enumerate_surface_formats:
                  return "failed_enumerate_surface_formats";
               case surface_support_error::failed_enumerate_surface_present_modes:
                  return "failed_enumerate_surface_present_modes";
               default:
                  return "UNKNOWN";
            }
         }
      };

      struct swapchain_error_category : std::error_category
      {
         const char* name() const noexcept override { return "vk_swapchain"; }
         std::string message(int err) const override
         {
            return instance::to_string(static_cast<instance::error>(err));
         }
      };

      const surface_support_error_category surface_support_error_cat;
      const swapchain_error_category swapchain_error_cat;

      std::error_code make_error_code(surface_support_error err)
      {
         return {static_cast<int>(err), surface_support_error_cat};
      }

      struct surface_support_details
      {
         VkSurfaceCapabilitiesKHR capabilities;
         dynamic_array<VkSurfaceFormatKHR> formats;
         dynamic_array<VkPresentModeKHR> present_modes;
      };

      detail::result<surface_support_details> query_surface_support(const device& dev)
      {
         auto* gpu = dev.phys_device.vk_physical_device;
         if (gpu == VK_NULL_HANDLE)
         {
            // clang-format off
            return monad::to_right(error{
               .type = make_error_code(surface_support_error::physical_device_handle_null)
            });
            // clang-format on
         }

         auto* surface = dev.phys_device.vk_surface;
         if (surface == VK_NULL_HANDLE)
         {
            // clang-format off
            return monad::to_right(error{
               .type = make_error_code(surface_support_error::surface_handle_null)
            });
            // clang-format on
         }

         surface_support_details details;

         {
            const auto res =
               vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details.capabilities);
            if (res != VK_SUCCESS)
            {
               // clang-format off
               return monad::to_right(error{
                  .type = make_error_code(surface_support_error::failed_get_surface_capabilities),
                  .result = res
               });
               // clang-format on
            }
         }

         // clang-format off
         const auto handle_err = [&](VkResult res, surface_support_error err)
         {
            return monad::to_right(error{
               .type = make_error_code(err),
               .result = res
            });
         };
         // clang-format on

         using namespace std::placeholders;
         return get_array<VkSurfaceFormatKHR>(vkGetPhysicalDeviceSurfaceFormatsKHR, gpu, surface)
            .join(
               [&](const auto& formats) -> result<surface_support_details> {
                  details.formats = formats;

                  return get_array<VkPresentModeKHR>(
                     vkGetPhysicalDeviceSurfacePresentModesKHR, gpu, surface)
                     .join(
                        [&](const auto& mode) -> result<surface_support_details> {
                           details.present_modes = mode;

                           return monad::to_left(details);
                        },
                        std::bind(handle_err, _1,
                           surface_support_error::failed_enumerate_surface_present_modes));
               },
               std::bind(handle_err, _1, surface_support_error::failed_enumerate_surface_formats));
      }
   } // namespace detail

   std::string swapchain::to_string(error err)
   {
      switch (err)
      {
         case error::surface_handle_not_provided:
            return "surface_handle_not_provided";
         default:
            return "UNKNOWN";
      }
   }

   std::error_code swapchain::make_error_code(error err)
   {
      return {static_cast<int>(err), detail::swapchain_error_cat};
   }

   swapchain_builder::swapchain_builder(const device& device, logger* const p_logger) :
      p_logger{p_logger}
   {
      info.physical_device = device.phys_device.vk_physical_device;
      info.surface = device.phys_device.vk_surface;
      info.device = device.vk_device;

      info.graphics_queue_index =
         device.get_queue_index(queue::type::graphics)
            .right_map([&](const detail::error& err) {
               LOG_ERROR_P(
                  p_logger, "Failed to retrieve graphics queue index: {1}", err.type.message());
               abort();

               return 0u;
            })
            .join();

      info.present_queue_index =
         device.get_queue_index(queue::type::present)
            .right_map([&](const detail::error& err) {
               LOG_ERROR_P(
                  p_logger, "Failed to retrieve present queue index: {1}", err.type.message());
               abort();

               return 0u;
            })
            .join();
   }

   detail::result<swapchain> swapchain_builder::build() const
   {
      if (info.surface == VK_NULL_HANDLE)
      {
         // clang-format off
         return monad::to_right(detail::error{
            .type = swapchain::make_error_code(swapchain::error::surface_handle_not_provided)
         });
         // clang-format on
      }
      return monad::to_right(detail::error{
         .type = swapchain::make_error_code(swapchain::error::surface_handle_not_provided)});
   }

   swapchain_builder& swapchain_builder::set_desired_format(VkSurfaceFormatKHR format) noexcept
   {
      info.formats.insert(info.formats.cbegin(), format);
      return *this;
   }
   swapchain_builder& swapchain_builder::add_fallback_format(VkSurfaceFormatKHR format) noexcept
   {
      info.formats.push_back(format);
      return *this;
   }
   swapchain_builder& swapchain_builder::use_default_format() noexcept
   {
      info.formats.clear();
      info.formats.push_back({VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
      info.formats.push_back({VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
      return *this;
   }

} // namespace core::vk
