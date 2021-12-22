#include "libash/detail/physical_device_rating.hpp"
#include <libash/physical_device.hpp>

#include <libmannele/core/types.hpp>

#include <magic_enum.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

namespace rv = ranges::views;
namespace ra = ranges::actions;

namespace ash::inline v0
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

   struct physical_device_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "ash::physical_device";
      }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return std::string(
            magic_enum::enum_name(static_cast<physical_device_selection_error>(err)));
      }
   };

   auto to_error_condition(physical_device_selection_error err) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(err), physical_device_error_category()});
   }

   /**
    * @brief give a rating to a physical device based on its properties. If the physical device
    * doesn't meet the requirements, it will have a rating of -1.
    */
   auto rate_physical_device(vk::PhysicalDevice device, physical_device_select_info&& select_info)
      -> physical_device_rating;

   auto find_supported_extensions(std::span<const vk::ExtensionProperties> properties,
                                  std::span<const char* const> names) -> std::vector<const char*>;

   auto find_most_suitable_gpu(physical_device_select_info&& info)
      -> reglisse::result<physical_device, runtime_error>
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
         const auto err_code = physical_device_selection_error::no_suitable_device_found;
         const auto err = runtime_error(to_error_condition(err_code));

         return reglisse::err(err);
      }

      return reglisse::ok(std::move(rated_devices[0]).device);
   }

   auto rate_physical_device(vk::PhysicalDevice device, physical_device_select_info&& select_info)
      -> physical_device_rating
   {
      using detail::rate_extension_support;
      using detail::rate_properties_support;
      using detail::rate_queue_support;
      using detail::tally_ratings;

      const auto features = device.getFeatures();
      const auto properties = device.getProperties();
      const auto memory_properties = device.getMemoryProperties();
      const auto queue_properties = device.getQueueFamilyProperties();
      const auto extensions = device.enumerateDeviceExtensionProperties();

      const auto [ext_to_enable, ext_rating] = rate_extension_support(extensions, select_info);
      const auto properties_rating = rate_properties_support(properties, select_info);
      const auto [queues, queue_rating] = rate_queue_support(queue_properties, select_info);

      const i32 rating = tally_ratings(ext_rating, properties_rating, queue_rating);

      return {.device = physical_device{.device = device,
                                        .features = features,
                                        .properties = properties,
                                        .memory_properties = memory_properties,
                                        .queues_to_create = queues,
                                        .extensions_to_enable = ext_to_enable},
              .rating = rating};
   }

   physical_device::operator vk::PhysicalDevice() const { return device; }
} // namespace ash::inline v0
