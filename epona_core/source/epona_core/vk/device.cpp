#include "epona_core/vk/device.hpp"
#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/details/logger.hpp"

#include <ranges>

namespace core::vk
{
   namespace detail
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
         const char* name() const noexcept override { return "vk_device"; }
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

      struct device_error_category : std::error_category
      {
         const char* name() const noexcept override { return "vk_device"; }
         std::string message(int err) const override
         {
            return device::to_string(static_cast<device::error>(err));
         }
      };

      const device_error_category device_err_cat;
   } // namespace detail

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

   detail::result<uint32_t> device::get_queue_index(queue::type type) const
   {
      using error = queue::error;

      if (type == queue::type::present)
      {
         const auto index = detail::get_present_queue_index(
            phys_device.vk_physical_device, phys_device.vk_surface, phys_device.queue_families);
         if (!index)
         {
            return monad::right_t<detail::error>{
               detail::make_error_code(error::present_unavailable)};
         }
         else
         {
            return monad::left_t<uint32_t>{.value = *index};
         }
      }
      else if (type == queue::type::graphics)
      {
         const auto index = detail::get_graphics_queue_index(phys_device.queue_families);
         if (!index)
         {
            return monad::right_t<detail::error>{
               detail::make_error_code(error::graphics_unavailable)};
         }
         else
         {
            return monad::left_t<uint32_t>{.value = *index};
         }
      }
      else if (type == queue::type::compute)
      {
         const auto index = detail::get_separated_compute_queue_index(phys_device.queue_families);
         if (!index)
         {
            return monad::right_t<detail::error>{
               detail::make_error_code(error::compute_unavailable)};
         }
         else
         {
            return monad::left_t<uint32_t>{.value = *index};
         }
      }
      else if (type == queue::type::transfer)
      {
         const auto index = detail::get_separated_transfer_queue_index(phys_device.queue_families);
         if (!index)
         {
            return monad::right_t<detail::error>{
               detail::make_error_code(queue::error::transfer_unavailable)};
         }
         else
         {
            return monad::left_t<uint32_t>{.value = *index};
         }
      }
      else
      {
         return monad::right_t<detail::error>{
            detail::make_error_code(queue::error::invalid_queue_family_index)};
      }
   }

   detail::result<uint32_t> device::get_dedicated_queue_index(queue::type type) const
   {
      if (type == queue::type::compute)
      {
         const auto index = detail::get_dedicated_compute_queue_index(phys_device.queue_families);
         if (!index)
         {
            return monad::right_t<detail::error>{
               detail::make_error_code(queue::error::compute_unavailable)};
         }
         else
         {
            return monad::left_t<uint32_t>{.value = *index};
         }
      }
      else if (type == queue::type::transfer)
      {
         const auto index = detail::get_dedicated_transfer_queue_index(phys_device.queue_families);
         if (!index)
         {
            return monad::right_t<detail::error>{
               detail::make_error_code(queue::error::transfer_unavailable)};
         }
         else
         {
            return monad::left_t<uint32_t>{.value = *index};
         }
      }
      else
      {
         return monad::right_t<detail::error>{
            detail::make_error_code(queue::error::invalid_queue_family_index)};
      }
   }

   detail::result<VkQueue> device::get_queue(queue::type type) const
   {
      // clang-format off
      return get_queue_index(type)
         .join(
            [&](uint32_t i) -> detail::result<VkQueue> {
               return monad::left_t<VkQueue>{get_queue(i)};
            },
            [](const detail::error& err) -> detail::result<VkQueue> {
               return monad::right_t<detail::error>{err};
            });
      // clang-format on
   }

   detail::result<VkQueue> device::get_dedicated_queue(queue::type type) const
   {
      // clang-format off
      return get_dedicated_queue_index(type)
         .join(
            [&](uint32_t i) -> detail::result<VkQueue> {
               return monad::left_t<VkQueue>{get_queue(i)};
            },
            [](const detail::error& err) -> detail::result<VkQueue> {
               return monad::right_t<detail::error>{err};
            });
      // clang-format on
   }

   VkQueue device::get_queue(uint32_t family) const noexcept
   {
      VkQueue out_queue{VK_NULL_HANDLE};
      vkGetDeviceQueue(vk_device, family, 0, &out_queue);

      return out_queue;
   }

   std::string device::to_string(error err)
   {
      switch (err)
      {
         case error::failed_create_device:
            return "failed_create_device";
         default:
            return "UNKNOWN";
      }
   }

   std::error_code device::make_error_code(error err)
   {
      return {static_cast<int>(err), detail::device_err_cat};
   }

   device_builder::device_builder(physical_device&& phys_device, logger* p_logger) :
      p_logger{p_logger}
   {
      info.phys_device = std::move(phys_device);
   }

   detail::result<device> device_builder::build()
   {
      dynamic_array<queue::description> descriptions;
      descriptions.insert(descriptions.cend(), info.queue_descriptions);

      if (descriptions.empty())
      {
         for (uint32_t i = 0; i < info.phys_device.queue_families.size(); ++i)
         {
            // clang-format off
            descriptions.push_back(
               {
                  .index = i, 
                  .count = 1, 
                  .priorities = dynamic_array<float>{1.0f}
               }
            );
            // clang-format on
         }
      }

      dynamic_array<VkDeviceQueueCreateInfo> queue_create_infos;
      queue_create_infos.reserve(descriptions.size());
      for (const auto& desc : descriptions)
      {
         // clang-format off
         queue_create_infos.push_back(
            {
               .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
               .pNext = nullptr,
               .flags = {},
               .queueFamilyIndex = desc.index,
               .queueCount = desc.count,
               .pQueuePriorities = desc.priorities.data()
            }
         );
         // clang-format on    
      }

      tiny_dynamic_array<const char*, 4> extensions{info.desired_extensions};
      if(info.phys_device.vk_surface != VK_NULL_HANDLE)
      {
         extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      } 

      VkPhysicalDevice vk_gpu = info.phys_device.vk_physical_device;

      uint32_t device_ext_count =0;
      vkEnumerateDeviceExtensionProperties(vk_gpu, nullptr, &device_ext_count, nullptr);
      dynamic_array<VkExtensionProperties> ext_properties(device_ext_count);
      vkEnumerateDeviceExtensionProperties(vk_gpu, nullptr, &device_ext_count, ext_properties.data());

      for (const auto& desired : extensions)
      {
         bool is_present = false;
         for (const auto& available : ext_properties)
         {
            if (strcmp(desired, available.extensionName) == 0)
            {
               is_present = true;
            }
         }

         if (!is_present)
         {
            return monad::right_t<detail::error>{
               device::make_error_code(device::error::device_extension_not_supported)};
         }
      }

      // clang-format off
      std::ranges::for_each(extensions, [&](const char* ext_name){
         LOG_INFO_P(p_logger, "Device extension: {1} - ENABLED", ext_name);
      });

      const VkDeviceCreateInfo device_create_info
      {
         .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
         .pNext = nullptr,
         .flags = {},
         .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
         .pQueueCreateInfos = queue_create_infos.data(),
         .enabledLayerCount = 0,
         .ppEnabledLayerNames = nullptr,
         .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
         .ppEnabledExtensionNames = extensions.data(),
         .pEnabledFeatures = &info.phys_device.features
      };
      // clang-format on

      VkDevice vk_device = VK_NULL_HANDLE;
      const VkResult res = vkCreateDevice(vk_gpu, &device_create_info, nullptr, &vk_device);
      if (res != VK_SUCCESS)
      {
         return monad::right_t<detail::error>{
            device::make_error_code(device::error::failed_create_device), res};
      }

      volkLoadDevice(vk_device);

      vk::device device = {};
      device.vk_device = vk_device;
      device.phys_device = std::move(info.phys_device);
      device.extensions = extensions;

      return monad::left_t<vk::device>{std::move(device)};
   }

   device_builder& device_builder::set_queue_setup(
      const dynamic_array<queue::description>& descriptions)
   {
      info.queue_descriptions = descriptions;
      return *this;
   }
} // namespace core::vk
