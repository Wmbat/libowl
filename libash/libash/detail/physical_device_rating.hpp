#ifndef LIBASH_DETAIL_PHYSICAL_DEVICE_RATING_HPP_
#define LIBASH_DETAIL_PHYSICAL_DEVICE_RATING_HPP_

#include <libash/core.hpp>
#include <libash/detail/vulkan.hpp>
#include <libash/physical_device.hpp>

namespace ash::inline v0
{
   namespace detail
   {
      /**
       * @brief
       *
       * @param[in] properties
       * @param[in] info
       *
       * @return
       */
      auto rate_properties_support(const vk::PhysicalDeviceProperties& properties,
                                   const physical_device_select_info& info) -> i32;
      /**
       * @brief
       *
       * @param[in] device_exts
       * @param[in] info
       *
       * @return
       */
      auto rate_extension_support(std::span<const vk::ExtensionProperties> device_extensions,
                                  const physical_device_select_info& info)
         -> std::pair<std::vector<std::string_view>, i32>;
      /**
       * @brief
       *
       * @param[in] queue_properties
       * @param[in] info
       *
       * @return
       */
      auto rate_queue_support(std::span<const vk::QueueFamilyProperties> queue_properties,
                              const physical_device_select_info& info)
         -> std::pair<std::vector<desired_queue_data>, i32>;
   } // namespace detail
} // namespace ash::inline v0

#endif // LIBASH_DETAIL_PHYSICAL_DEVICE_RATING_HPP_
