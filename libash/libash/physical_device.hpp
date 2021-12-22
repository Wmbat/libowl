/**
 * @file libash/physical_device.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBASH_PHYSICAL_DEVICE_HPP_
#define LIBASH_PHYSICAL_DEVICE_HPP_

#include <libash/instance.hpp>
#include <libash/runtime_error.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <libreglisse/result.hpp>

namespace ash::inline v0
{
   enum struct physical_device_type
   {
      other = 0,
      integrated = 1,
      discrete = 2,
      virtual_gpu = 3,
      cpu = 4
   };

   enum struct physical_device_selection_error
   {
      no_suitable_device_found
   };

   struct physical_device_select_info
   {
      ash::instance& instance;

      vk::SurfaceKHR surface;

      physical_device_type prefered_type = physical_device_type::discrete;

      bool allow_any_physical_device_type = true;

      bool require_transfer_queue = false;
      bool require_compute_queue = false;

      mannele::semantic_version desired_version = {1, 0, 0};
      mannele::semantic_version minimum_version = {1, 0, 0};

      std::vector<const char*> required_extensions;
      std::vector<const char*> desired_extensions;
   };

   struct desired_queue_data
   {
      vk::QueueFlags flags;
      u32 family;
      u32 index;
   };

   struct physical_device
   {
      vk::PhysicalDevice device; // NOLINT

      vk::PhysicalDeviceFeatures features;                  // NOLINT
      vk::PhysicalDeviceProperties properties;              // NOLINT
      vk::PhysicalDeviceMemoryProperties memory_properties; // NOLINT

      std::vector<desired_queue_data> queues_to_create; // NOLINT

      std::vector<std::string_view> extensions_to_enable; // NOLINT

      operator vk::PhysicalDevice() const;
   };

   /**
    * @brief Finds a physical device that matches the selection information as well as possible. If
    * no physical device could be found, the result will hold an error.
    *
    * @param [in] info Information used for selection the best suited physical device.
    *
    * @return
    */
   auto find_most_suitable_gpu(physical_device_select_info&& info)
      -> reglisse::result<physical_device, runtime_error>;
} // namespace ash::inline v0

#endif // LIBASH_PHYSICAL_DEVICE_HPP_
