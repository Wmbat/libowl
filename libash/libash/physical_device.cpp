#include <libash/physical_device.hpp>

#include <libmannele/core/types.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

namespace rv = ranges::views;
namespace ra = ranges::actions;

namespace ash
{
   static constexpr i32 supported_extension_value = 1;
   static constexpr i32 preferred_api_version_value = 500;
   static constexpr i32 minimum_api_version_value = 250;
   static constexpr i32 prefered_physical_device_type_value = 1000;
   static constexpr i32 discrete_physical_device_type_value = 500;
   static constexpr i32 integrated_physical_device_type_value = 400;
   static constexpr i32 virtual_physical_device_type_value = 300;
   static constexpr i32 cpu_physical_device_type_value = 200;
   static constexpr i32 other_physical_device_type_value = 100;

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
    * @brief give a rating to a physical device based on its properties. If the physical device
    * doesn't meet the requirements, it will have a rating of -1.
    */
   auto rate_physical_device(vk::PhysicalDevice device, physical_device_select_info&& select_info)
      -> physical_device_rating;

   auto find_supported_extensions(std::span<const vk::ExtensionProperties> properties,
                                  std::span<const char* const> names) -> std::vector<const char*>;

   auto find_most_suitable_gpu(physical_device_select_info&& info)
      -> reglisse::result<physical_device, physical_device_selection_error>
   {
      vk::Instance instance = info.instance;

      const auto rate_device = [&](vk::PhysicalDevice device) {
         return rate_physical_device(device, std::move(info));
      };

      // clang-format off

      const std::vector raw_devices = instance.enumeratePhysicalDevices();
      std::vector rated_devices = raw_devices 
         | rv::transform(rate_device)
         | ranges::to_vector 
         | ra::sort(ranges::greater(), &physical_device_rating::rating);

      // clang-format on

      if (std::size(rated_devices) == 0 || rated_devices[0].rating < 0)
      {
         return reglisse::err(physical_device_selection_error::no_suitable_device_found);
      }

      return reglisse::ok(std::move(rated_devices[0]).device);
   }

   auto rate_properties(const vk::PhysicalDeviceProperties& properties,
                        const physical_device_select_info& info) -> i32;
   auto rate_extension_support(std::span<const vk::ExtensionProperties> device_exts,
                               const physical_device_select_info& info)
      -> std::pair<std::vector<const char*>, i32>;

   auto rate_physical_device(vk::PhysicalDevice device, physical_device_select_info&& select_info)
      -> physical_device_rating
   {
      const auto features = device.getFeatures();
      const auto properties = device.getProperties();
      const auto memory_properties = device.getMemoryProperties();
      const auto queue_properties = device.getQueueFamilyProperties();
      const auto extensions = device.enumerateDeviceExtensionProperties();

      const auto [ext_to_enable, ext_rating] = rate_extension_support(extensions, select_info);
      const auto properties_rating = rate_properties(properties, select_info);

      const i32 rating = properties_rating + ext_rating;

      return {.device = physical_device{.device = device,
                                        .surface = std::move(select_info.surface),
                                        .features = features,
                                        .properties = properties,
                                        .memory_properties = memory_properties,
                                        .extensions_to_enable = ext_to_enable},
              .rating = rating};
   }

   auto rate_property_api_version(const vk::PhysicalDeviceProperties& properties,
                                  const physical_device_select_info& info) -> i32
   {
      const auto device_version =
         mannele::semantic_version{.major = VK_VERSION_MAJOR(properties.apiVersion),
                                   .minor = VK_VERSION_MINOR(properties.apiVersion),
                                   .patch = VK_VERSION_PATCH(properties.apiVersion)};

      if (info.minimum_version > device_version)
      {
         return -1;
      }

      if (info.desired_version >= device_version)
      {
         return preferred_api_version_value;
      }

      return minimum_api_version_value;
   }

   auto rate_property_device_type(const vk::PhysicalDeviceProperties& properties,
                                  const physical_device_select_info& info) -> i32
   {
      i32 rating = 0;

      if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
      {
         const i32 preliminary = info.prefered_type == physical_device_type::discrete
                                    ? prefered_physical_device_type_value
                                    : discrete_physical_device_type_value;

         rating += preliminary;
      }
      else if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
      {
         rating += info.prefered_type == physical_device_type::integrated
                      ? prefered_physical_device_type_value
                      : integrated_physical_device_type_value;
      }
      else if (properties.deviceType == vk::PhysicalDeviceType::eVirtualGpu)
      {
         rating += info.prefered_type == physical_device_type::virtual_gpu
                      ? prefered_physical_device_type_value
                      : virtual_physical_device_type_value;
      }
      else if (properties.deviceType == vk::PhysicalDeviceType::eCpu)
      {
         rating += info.prefered_type == physical_device_type::cpu
                      ? prefered_physical_device_type_value
                      : cpu_physical_device_type_value;
      }
      else
      {
         rating += info.prefered_type == physical_device_type::other
                      ? prefered_physical_device_type_value
                      : other_physical_device_type_value;
      }

      return rating;
   }

   auto rate_properties(const vk::PhysicalDeviceProperties& properties,
                        const physical_device_select_info& info) -> i32
   {
      const i32 api_version_rating = rate_property_api_version(properties, info);
      const i32 device_type_rating = rate_property_device_type(properties, info);

      if (api_version_rating == -1 || device_type_rating == -1)
      {
         return -1;
      }

      return api_version_rating + device_type_rating;
   }

   auto rate_extension_support(std::span<const vk::ExtensionProperties> device_exts,
                               const physical_device_select_info& info)
      -> std::pair<std::vector<const char*>, i32>
   {
      const auto supported_required_ext =
         find_supported_extensions(device_exts, info.required_extensions);

      if (std::size(supported_required_ext) < std::size(info.required_extensions))
      {
         return {{}, -1};
      }

      const auto supported_desired_ext =
         find_supported_extensions(device_exts, info.desired_extensions);

      const std::vector ext_to_enable =
         rv::concat(info.required_extensions, supported_desired_ext) | ranges::to_vector;
      const i32 rating =
         static_cast<i32>(std::size(supported_desired_ext)) * supported_extension_value;

      return {ext_to_enable, rating};
   }

   auto find_supported_extensions(std::span<const vk::ExtensionProperties> properties,
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

         if (is_found)
         {
            unsupported.push_back(name);
         }
      }

      return unsupported;
   }

   physical_device::operator vk::PhysicalDevice() const { return device; }
} // namespace ash
