#include <libash/detail/physical_device_rating.hpp>

#include <libash/queue.hpp>

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

#include <ranges>

namespace rv = ranges::views;

namespace ash::inline v0
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

   namespace detail
   {
      auto find_supported_extensions(std::span<const vk::ExtensionProperties> properties,
                                     std::span<const char* const> names)
         -> std::vector<const char*>;

      auto rate_property_api_version(const vk::PhysicalDeviceProperties& properties,
                                     const physical_device_select_info& info) -> i32;
      auto rate_property_device_type(const vk::PhysicalDeviceProperties& properties,
                                     const physical_device_select_info& info) -> i32;

      auto rate_properties_support(const vk::PhysicalDeviceProperties& properties,
                                   const physical_device_select_info& info) -> i32
      {
         const i32 api_version_rating = rate_property_api_version(properties, info);
         const i32 device_type_rating = rate_property_device_type(properties, info);

         return tally_ratings(api_version_rating, device_type_rating);
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

      auto rate_queue_support(std::span<const vk::QueueFamilyProperties> queue_properties,
                              const physical_device_select_info& info) -> i32
      {
         using detail::to_queue_info;

         // clang-format off
         const auto queue_infos = rv::enumerate(queue_properties) | rv::transform(to_queue_info);
         const auto supports_compute = queue_infos 
            | rv::filter(detail::does_queue_support_compute) 
            | ranges::to_vector;
         const auto supports_transfer = queue_infos 
            | rv::filter(detail::does_queue_support_transfer) 
            | ranges::to_vector;
         // clang-format on

         i32 compute_queue_rating = 0;
         i32 transfer_queue_rating = 0;

         if (info.require_compute_queue and supports_compute.empty())
         {
            compute_queue_rating = -1;
         }

         if (info.require_transfer_queue and supports_transfer.empty())
         {
            transfer_queue_rating = -1;
         }

         {
            const auto dedicated_it =
               ranges::find_if(supports_compute, detail::is_queue_dedicated_to_compute);
            if (dedicated_it != std::end(supports_compute))
            {
               compute_queue_rating = 10;
            }
            else
            {
               compute_queue_rating = 5;
            }
         }

         {
            const auto dedicated_it =
               ranges::find_if(supports_transfer, detail::is_queue_dedicated_to_transfer);
            if (dedicated_it != std::end(supports_transfer))
            {
               transfer_queue_rating = 10;
            }
            else
            {
               transfer_queue_rating = 5;
            }
         }

         return tally_ratings(compute_queue_rating, transfer_queue_rating);
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

   } // namespace detail
} // namespace ash::inline v0
