/**
 * @file 
 * @author  wmbat-dev@protonmail.com
 * @date 
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBASH_PHYSICAL_DEVICE_HPP_
#define LIBASH_PHYSICAL_DEVICE_HPP_

#include <libash/instance.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <libreglisse/result.hpp>

namespace ash
{
   enum struct physical_device_type
   {
      other = 0,
      integrated = 1,
      discrete = 2,
      virtual_gpu = 3,
      cpu = 4
   };

   struct physical_device_select_info
   {
      ash::instance& instance;

      vk::UniqueSurfaceKHR surface;

      physical_device_type prefered_type = physical_device_type::discrete;

      bool allow_any_physical_device_type = true;

      bool require_transfer_queue = false;
      bool require_compute_queue = false;

      mannele::semantic_version desired_version = {1, 0, 0};
      mannele::semantic_version minimum_version = {1, 0, 0};

      std::vector<const char*> required_extensions;
      std::vector<const char*> desired_extensions;
   };

   struct physical_device
   {
      vk::PhysicalDevice device;
      vk::UniqueSurfaceKHR surface;

      vk::PhysicalDeviceFeatures features;
      vk::PhysicalDeviceProperties properties;
      vk::PhysicalDeviceMemoryProperties memory_properties;

      std::vector<const char*> extensions_to_enable;

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
   auto find_most_suitable_gpu(const physical_device_select_info& info)
      -> reglisse::result<physical_device, u32>;
} // namespace ash

#endif // LIBASH_PHYSICAL_DEVICE_HPP_
