#include <libash/device.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/transform.hpp>

#include <spdlog/spdlog.h>

namespace rv = ranges::views;

namespace ash::inline v0
{
   namespace
   {
      /**
       * @brief Convert a string_view into a C string
       *
       * @param[in] str
       *
       * @return
       */
      auto to_cstring(std::string_view str) -> const char* { return str.data(); }

      struct queue_priority_info
      {
         vk::QueueFlags flags;
         u32 family;
         u32 count;
         std::vector<f32> priorities;
      };

      auto to_queue_priority_info(const std::pair<u64, vk::QueueFamilyProperties>& pair)
         -> queue_priority_info
      {
         u32 const count = pair.second.queueCount;
         return {.flags = pair.second.queueFlags,
                 .family = static_cast<u32>(pair.first),
                 .count = count,
                 .priorities = std::vector(count, 0.0f)};
      }

      auto to_device_queue_create_info(const queue_priority_info& info) -> vk::DeviceQueueCreateInfo
      {
         return vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(info.family)
            .setQueueCount(info.count)
            .setQueuePriorities(info.priorities);
      }

      auto create_vulkan_device(const device_create_info& info) -> vk::UniqueDevice
      {
         auto const& features = info.physical.features;

         // clang-format off
         const auto extension_names = info.extensions
            | rv::transform(to_cstring) 
            | ranges::to<std::vector>;
   
         auto const family_properties = info.physical.device.getQueueFamilyProperties();
         auto const temporary_queue_info = family_properties
            | rv::enumerate
            | rv::transform(to_queue_priority_info) 
            | ranges::to_vector;
         auto const queue_create_info = temporary_queue_info
            | rv::transform(to_device_queue_create_info)
            | ranges::to_vector;
         // clang-format on

         return info.physical.device.createDeviceUnique(
            vk::DeviceCreateInfo()
               .setPEnabledExtensionNames(extension_names)
               .setQueueCreateInfos(queue_create_info)
               .setPEnabledFeatures(&features));
      }
      auto select_queues(const device_create_info& info, vk::Device device) -> std::vector<queue>
      {
         auto const to_queue = [device](const desired_queue_data& data) {
            return queue{.value = device.getQueue(data.family, data.index),
                         .type = data.flags,
                         .family_index = data.family,
                         .queue_index = data.index};
         };

         return info.desired_queues | rv::transform(to_queue) | ranges::to<std::vector>;
      }
   } // namespace

   device::device(device_create_info&& info) :
      mp_logger(&info.logger), m_api_version(), m_device(create_vulkan_device(info)),
      m_queues(select_queues(info, m_device.get()))
   {
      mp_logger->debug("vulkan logical device created");
      for (const auto& queue : m_queues)
      {
         mp_logger->debug("queue = {}", queue);
      }
   }

   auto device::api_version() const noexcept -> mannele::semantic_version { return m_api_version; }

   device::operator vk::Device() const noexcept { return m_device.get(); }

   auto operator==(device const& lhs, device const& rhs) noexcept -> bool
   {
      if (&lhs != &rhs)
      {
         bool const version_check = lhs.m_api_version == rhs.m_api_version;
         bool const device_check = lhs.m_device.get() == rhs.m_device.get();
         bool const queue_check = lhs.m_queues == rhs.m_queues;

         return version_check && device_check && queue_check;
      }
      else
      {
         return true;
      }
   }
} // namespace ash::inline v0
