#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/defines.hpp"
#include "epona_core/vk/details/includes.hpp"
#include "epona_core/vk/physical_device.hpp"

namespace core::vk
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
   public:
      device() = default;
      device(const device& other) = delete;
      device(device&& other) noexcept;
      ~device();

      device& operator=(const device& other) = delete;
      device& operator=(device&& other);

      details::result<uint32_t> get_queue_index(queue::type type) const;
      details::result<uint32_t> get_dedicated_queue_index(queue::type type) const;

      details::result<VkQueue> get_queue(queue::type type) const;
      details::result<VkQueue> get_dedicated_queue(queue::type type) const;

   private:
      VkQueue get_queue(uint32_t family) const noexcept;

   public:
      VkDevice vk_device{VK_NULL_HANDLE};
      physical_device phys_device;
   };

   class device_builder
   {
   public:
      device_builder(physical_device&& phys_device);

      details::result<device> build();

      device_builder& set_queue_setup(const dynamic_array<queue::description>& descriptions);

   private:
      struct info
      {
         physical_device phys_device;

         dynamic_array<queue::description> queue_descriptions;
      } info;
   };
} // namespace core::vk
