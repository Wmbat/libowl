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

#include <tl/expected.hpp>

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

   struct desired_queue_data
   {
      vk::QueueFlags flags;
      u32 family;
      u32 index;
   };

   struct physical_device
   {
      vk::PhysicalDevice device;

      vk::PhysicalDeviceFeatures features;
      vk::PhysicalDeviceProperties properties;
      vk::PhysicalDeviceMemoryProperties memory_properties;

      std::vector<vk::QueueFamilyProperties> queue_properties;
      std::vector<vk::ExtensionProperties> extension_properties;

      operator vk::PhysicalDevice() const;
   };

   /**
    * @brief
    *
    * @param[in] instance
    *
    * @return A list of physical device objects
    */
   auto enumerate_physical_devices(ash::instance const& instance) -> std::vector<physical_device>;

   /**
    * @brief
    */
   struct physical_device_selection_results
   {
      physical_device const* p_physical_device;

      std::vector<desired_queue_data> queues_to_create;
      std::vector<std::string_view> extension_to_enable;
   };

   /**
    * @brief Struct used to group up all criteria that can be used to rate the viability of specific
    * physical device.
    */
   struct physical_device_selection_criteria
   {
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

   /**
    * @brief
    *
    * @param [in] physical_devices A list of all available physical devices to choose from.
    * @param [in] info The criteria used for selection the best suited physical device.
    *
    * @return 
    */
   auto find_most_suitable_physical_device(std::span<const physical_device> physical_devices,
                                           physical_device_selection_criteria&& info)
      -> tl::expected<physical_device_selection_results, runtime_error>;
} // namespace ash::inline v0

#endif // LIBASH_PHYSICAL_DEVICE_HPP_
