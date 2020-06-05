#include "epona_core/vk/device.hpp"

namespace core::vk
{
   namespace details
   {
      std::string to_string(queue::error err)
      {
         switch (err)
         {
            case queue::error::present_unavailable:
               return "present_unavailable";
            case queue::error::graphics_unavailable:
               return "graphics_unavailable";
            case queue::error::compute_unavailable:
               return "compute_unavailable";
            case queue::error::transfer_unavailable:
               return "transfer_unavailable";
            case queue::error::queue_index_out_of_range:
               return "queue_index_out_of_range";
            case queue::error::invalid_queue_family_index:
               return "invalid_queue_family_index";
            default:
               return "UNKNOWN";
         }
      }

      struct queue_error_category : std::error_category
      {
         const char* name() const noexcept override { return "vk_physical_device"; }
         std::string message(int err) const override
         {
            return to_string(static_cast<queue::error>(err));
         }
      };

      const queue_error_category queue_err_cat;

      std::error_code make_error_code(queue::error err)
      {
         return {static_cast<int>(err), queue_err_cat};
      }
   } // namespace details

   device::device(device&& other) noexcept { *this = std::move(other); }
   device::~device()
   {
      if (vk_device != VK_NULL_HANDLE)
      {
         vkDestroyDevice(vk_device, nullptr);
         vk_device = VK_NULL_HANDLE;
      }
   }

   device& device::operator=(device&& rhs)
   {
      if (this == &rhs)
      {
         return *this;
      }

      vk_device = rhs.vk_device;
      rhs.vk_device = VK_NULL_HANDLE;

      phys_device = std::move(rhs.phys_device);

      return *this;
   }

   details::result<uint32_t> device::get_queue_index(queue::type type) const
   {
      using uint_opt = const std::optional<uint32_t>;

      if (type == queue::type::present)
      {
         uint_opt index = details::get_present_queue_index(
            phys_device.vk_physical_device, phys_device.vk_surface, phys_device.queue_families);
         if (!index)
         {
            return details::make_error_code(queue::error::present_unavailable);
         }
         else
         {
            return details::result<uint32_t>(index.value());
         }
      }
      else if (type == queue::type::graphics)
      {
         uint_opt index = details::get_graphics_queue_index(phys_device.queue_families);
         if (!index)
         {
            return details::make_error_code(queue::error::graphics_unavailable);
         }
         else
         {
            return details::result<uint32_t>(index.value());
         }
      }
      else if (type == queue::type::compute)
      {
         uint_opt index = details::get_separated_compute_queue_index(phys_device.queue_families);
         if (!index)
         {
            return details::make_error_code(queue::error::compute_unavailable);
         }
         else
         {
            return details::result<uint32_t>(index.value());
         }
      }
      else if (type == queue::type::transfer)
      {
         uint_opt index = details::get_separated_transfer_queue_index(phys_device.queue_families);
         if (!index)
         {
            return details::make_error_code(queue::error::transfer_unavailable);
         }
         else
         {
            return details::result<uint32_t>(index.value());
         }
      }
      else
      {
         return details::make_error_code(queue::error::invalid_queue_family_index);
      }
   }

   details::result<uint32_t> device::get_dedicated_queue_index(queue::type type) const
   {
      using uint_opt = const std::optional<uint32_t>;

      if (type == queue::type::compute)
      {
         uint_opt index = details::get_dedicated_compute_queue_index(phys_device.queue_families);
         if (!index)
         {
            return details::make_error_code(queue::error::compute_unavailable);
         }
         else
         {
            return details::result<uint32_t>(index.value());
         }
      }
      else if (type == queue::type::transfer)
      {
         uint_opt index = details::get_dedicated_transfer_queue_index(phys_device.queue_families);
         if (!index)
         {
            return details::make_error_code(queue::error::transfer_unavailable);
         }
         else
         {
            return details::result<uint32_t>(index.value());
         }
      }
      else
      {
         return details::make_error_code(queue::error::invalid_queue_family_index);
      }
   }

   details::result<VkQueue> device::get_queue(queue::type type) const
   {
      auto index = get_queue_index(type);
      if (!index)
      {
         return index.error_type();
      }
      else
      {
         return details::result(get_queue(index.value()));
      }
   }

   details::result<VkQueue> device::get_dedicated_queue(queue::type type) const
   {
      auto index = get_dedicated_queue_index(type);
      if (!index)
      {
         return index.error_type();
      }
      else
      {
         return details::result(get_queue(index.value()));
      }
   }

   VkQueue device::get_queue(uint32_t family) const noexcept
   {
      VkQueue out_queue{VK_NULL_HANDLE};
      vkGetDeviceQueue(vk_device, family, 0, &out_queue);

      return out_queue;
   }

   device_builder::device_builder(physical_device&& phys_device)
   {
      info.phys_device = std::move(phys_device);
   }

   /*
   details::result<device> device_builder::build() {}
   */

   device_builder& device_builder::set_queue_setup(
      const dynamic_array<queue::description>& descriptions)
   {
      info.queue_descriptions = descriptions;
      return *this;
   }
} // namespace core::vk
