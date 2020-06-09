#pragma once

#include "epona_core/containers/dynamic_array.hpp"
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
      enum class error
      {
         device_extension_not_supported,
         failed_create_device
      };

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

      static std::string to_string(error err);
      static std::error_code make_error_code(error err);

   private:
      VkQueue get_queue(uint32_t family) const noexcept;

   public:
      VkDevice vk_device{VK_NULL_HANDLE};
      physical_device phys_device;

      dynamic_array<const char*> extensions;
   };

   class device_builder
   {
   public:
      device_builder(physical_device&& phys_device, logger* p_logger = nullptr);

      details::result<device> build();

      device_builder& set_queue_setup(const dynamic_array<queue::description>& descriptions);
      device_builder& add_desired_extension(const std::string& extension_name);

   private:
      logger* p_logger;

      struct info
      {
         physical_device phys_device;

         dynamic_array<queue::description> queue_descriptions;

         dynamic_array<const char*> desired_extensions;
      } info;
   };
} // namespace core::vk
