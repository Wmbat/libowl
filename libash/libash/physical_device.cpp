#include <libash/physical_device.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

namespace rv = ranges::views;
namespace ra = ranges::actions;

namespace ash
{
   struct physical_device_decs
   {
      vk::PhysicalDevice device;

      vk::PhysicalDeviceFeatures features;
      vk::PhysicalDeviceProperties properties;
      vk::PhysicalDeviceMemoryProperties memory_properties;

      std::vector<vk::QueueFamilyProperties> queue_properties;
      std::vector<vk::ExtensionProperties> extension_properties;
   };

   struct physical_device_rating
   {
      physical_device device;
      i32 rating;
   };

   /**
    * @brief Convert a vulkan physical device handle to a physical device description containing
    * relevant information such as the device features, properties, memory properties as well as
    * queue information and extensions supported.
    *
    * @param [in] device The physical device to convert
    *
    * @return A populated physical_device_decs struct
    */
   auto to_physical_device_desc(vk::PhysicalDevice device) -> physical_device_decs;
   /**
    * @brief give a rating to a physical device based on its properties. If the physical device
    * doesn't meet the requirements, it will have a rating of -1.
    */
   auto rate_physical_device(const physical_device_decs& desc) -> physical_device_rating;

   auto find_unsupported_extensions(std::span<const vk::ExtensionProperties> properties,
                                    std::span<const char* const> names) -> std::vector<const char*>;

   physical_device::operator vk::PhysicalDevice() const { return device; }

   auto find_most_suitable_gpu(const physical_device_select_info& info)
      -> reglisse::result<physical_device, u32>
   {
      vk::Instance instance = info.instance;

      // clang-format off

      const std::vector raw_devices = instance.enumeratePhysicalDevices();
      const std::vector rated_devices = raw_devices 
         | rv::transform(to_physical_device_desc) 
         | rv::transform(rate_physical_device)
         | ranges::to_vector 
         | ra::sort(ranges::greater(), &physical_device_rating::rating);

      // clang-format on

      if (std::size(rated_devices) == 0)
      {
         // error
      }
   }

   auto to_physical_device_desc(vk::PhysicalDevice device) -> physical_device_decs
   {
      return {.device = device,
              .features = device.getFeatures(),
              .properties = device.getProperties(),
              .memory_properties = device.getMemoryProperties(),
              .queue_properties = device.getQueueFamilyProperties(),
              .extension_properties = device.enumerateDeviceExtensionProperties()};
   }

   auto rate_physical_device(const physical_device_decs& desc) -> physical_device_rating {}

   auto find_unsupported_extensions(std::span<const vk::ExtensionProperties> properties,
                                    std::span<const char* const> names) -> std::vector<const char*>
   {
      std::vector<const char*> unsupported;
      unsupported.reserve(std::size(names));

      for (const char* name : names)
      {
         bool is_found = false;
         for (const auto& property : properties)
         {
            if (std::string_view(property.extensionName) == std::string_view(name))
            {
               is_found = true;
            }
         }

         if (!is_found)
         {
            unsupported.push_back(name);
         }
      }

      return unsupported;
   }
} // namespace ash
