#ifndef LIBASH_DETAIL_PHYSICAL_DEVICE_RATING_HPP_
#define LIBASH_DETAIL_PHYSICAL_DEVICE_RATING_HPP_

#include <libash/core.hpp>
#include <libash/detail/vulkan.hpp>
#include <libash/physical_device.hpp>

namespace ash::inline v0
{
   namespace detail
   {
      auto tally_ratings(std::signed_integral auto... values) -> decltype((... + values))
      {
         if ((... || (-1 == values)))
         {
            return -1;
         }
         else
         {
            return (... + values);
         }
      }

      auto rate_properties_support(const vk::PhysicalDeviceProperties& properties,
                                   const physical_device_select_info& info) -> i32;
      auto rate_extension_support(std::span<const vk::ExtensionProperties> device_exts,
                                  const physical_device_select_info& info)
         -> std::pair<std::vector<const char*>, i32>;
      auto rate_queue_support(std::span<const vk::QueueFamilyProperties> queue_properties,
                              const physical_device_select_info& info) -> i32;
   } // namespace detail
} // namespace ash::inline v0

#endif // LIBASH_DETAIL_PHYSICAL_DEVICE_RATING_HPP_
