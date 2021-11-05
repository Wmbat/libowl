#include <libash/physical_device.hpp>

namespace ash
{
   struct physical_device_decs
   {
      vk::PhysicalDeviceFeatures features;
      vk::PhysicalDeviceProperties properties;
      vk::PhysicalDeviceMemoryProperties memory_properties;

      std::vector<vk::QueueFamilyProperties> queue_properties;
      std::vector<vk::ExtensionProperties> extension_properties;
   };

   auto to_physical_device_desc(vk::PhysicalDevice device) -> physical_device_decs
   {
      return {.features = device.getFeatures(),
              .properties = device.getProperties(),
              .memory_properties = device.getMemoryProperties(),
              .queue_properties = device.getQueueFamilyProperties(),
              .extension_properties = device.enumerateDeviceExtensionProperties()};
   }

   auto check_extension_support(std::span<const vk::ExtensionProperties> properties,
                                std::span<const char* const> names) -> std::vector<const char*>
   {}

   physical_device::operator vk::PhysicalDevice() const { return device; }

   auto find_most_suitable_gpu(const physical_device_select_info& info)
      -> reglisse::result<physical_device, u32>
   {
      vk::Instance instance = info.instance;

      for (vk::PhysicalDevice device : instance.enumeratePhysicalDevices())
      {
         auto device_desc = to_physical_device_desc(device);

         std::vector test1 =
            check_extension_support(device_desc.extension_properties, info.required_extensions);
         std::vector test2 =
            check_extension_support(device_desc.extension_properties, info.desired_extensions);
      }
   }

   auto check_extension_support(std::span<const vk::ExtensionProperties> properties,
                                std::span<const char*> names) -> std::vector<const char*>
   {
      std::vector<const char*> unsupported;
      unsupported.reserve(std::size(names));

      for (const char* name : names)
      {
         for (const auto& property : properties)
         {
         }
      }

      return unsupported;
   }
} // namespace ash
