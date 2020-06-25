/**
 * @file device.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 23rd of June, 2020
 * @copyright MIT License
 */

#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/graphics/vkn/core.hpp"
#include "epona_core/graphics/vkn/physical_device.hpp"

namespace core::gfx::vkn
{
   namespace queue
   {
      enum class type
      {
         present,
         graphics,
         compute,
         transfer
      };

      enum class error
      {
         present_unavailable,
         graphics_unavailable,
         compute_unavailable,
         transfer_unavailable,
         queue_index_out_of_range,
         invalid_queue_family_index
      };

      struct description
      {
         uint32_t index = 0;
         uint32_t count = 0;
         dynamic_array<float> priorities;
      };
   } // namespace queue

   struct device
   {
      enum class error
      {
         device_extension_not_supported,
         failed_to_create_device
      };

      device() = default;
      device(const device&) = delete;
      device(device&&);
      ~device();

      device& operator=(const device&) = delete;
      device& operator=(device&&);

      device&& set_physical_device(physical_device&& phys);
      device&& set_device(vk::Device&& device);
      device&& set_extensions(dynamic_array<const char*> extensions);

      result<uint32_t> get_queue_index(queue::type type) const;
      result<uint32_t> get_dedicated_queue_index(queue::type type) const;

      result<vk::Queue> get_queue(queue::type) const;
      result<vk::Queue> get_dedicated_queue(queue::type type) const;

      physical_device phys_device;

      vk::Device h_device;

      dynamic_array<const char*> extensions;
   };

   class device_builder
   {
   public:
      device_builder(
         const loader& vk_loader, physical_device&& phys_device, logger* const p_logger = nullptr);

      result<device> build();

      device_builder& set_queue_setup(const range_over<queue::description> auto& descriptions);
      device_builder& add_desired_extension(const std::string& extension_name);

   private:
      const loader& vk_loader;

      logger* const p_logger;

      struct info
      {
         physical_device phys_device;

         dynamic_array<queue::description> queue_descriptions;
         dynamic_array<const char*> desired_extensions;
      } info;
   };

} // namespace core::gfx::vkn
