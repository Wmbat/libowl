#include <libash/physical_device.hpp>

#include <libash/detail/physical_device_rating.hpp>

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
   struct physical_device_rating
   {
      physical_device_selection_results results;
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
   auto rate_physical_device(physical_device const& device,
                             physical_device_selection_criteria&& select_info)
      -> physical_device_rating;

   auto find_supported_extensions(std::span<const vk::ExtensionProperties> properties,
                                  std::span<const char* const> names) -> std::vector<const char*>;

   auto find_most_suitable_physical_device(std::span<const physical_device> physical_devices,
                                           physical_device_selection_criteria&& info)
      -> tl::expected<physical_device_selection_results, runtime_error>
   {
      assert(info.minimum_version <= info.desired_version); // NOLINT

      const auto rate_device = [&](physical_device const& device) {
         return rate_physical_device(device, std::move(info));
      };

      // clang-format off

      std::vector rated_devices = physical_devices
         | rv::transform(rate_device)
         | ranges::to_vector 
         | ra::sort(ranges::greater(), &physical_device_rating::rating);

      // clang-format on

      if (std::size(rated_devices) == 0)
      {
         const auto err_code = physical_device_selection_error::no_device_found;
         const auto err =
            runtime_error(to_error_condition(err_code), "No physical devices were found", {});

         return tl::unexpected(err);
      }

      if (rated_devices[0].rating < 0)
      {
         const auto err_code = physical_device_selection_error::no_suitable_device_found;
         const auto err = runtime_error(to_error_condition(err_code),
                                        "No physical device supported the desired properties", {});

         return tl::unexpected(err);
      }

      return rated_devices[0].results;
   }

   auto rate_physical_device(physical_device const& device,
                             physical_device_selection_criteria&& select_info)
      -> physical_device_rating
   {
      using detail::rate_extension_support;
      using detail::rate_properties_support;
      using detail::rate_queue_support;
      using detail::tally_ratings;

      std::span ext_properties = device.extension_properties;
      std::span queue_properties = device.queue_properties;

      auto const properties_rating = rate_properties_support(device.properties, select_info);
      auto const [ext_to_enable, ext_rating] = rate_extension_support(ext_properties, select_info);
      auto const [queues, queue_rating] = rate_queue_support(queue_properties, select_info);

      const i32 rating = tally_ratings(ext_rating, properties_rating, queue_rating);

      return {.results = {.p_physical_device = &device,
                          .queues_to_create = queues,
                          .extension_to_enable = ext_to_enable},
              .rating = rating};
   }

   physical_device::operator vk::PhysicalDevice() const { return device; }

   auto extract_physical_device_information(vk::PhysicalDevice device) -> physical_device
   {
      return {.device = device,
              .features = device.getFeatures(),
              .properties = device.getProperties(),
              .memory_properties = device.getMemoryProperties(),
              .queue_properties = device.getQueueFamilyProperties(),
              .extension_properties = device.enumerateDeviceExtensionProperties()};
   }

   auto enumerate_physical_devices(ash::instance const& instance) -> std::vector<physical_device>
   {
      vk::Instance const instance_handle = instance;
      std::vector const raw_devices = instance_handle.enumeratePhysicalDevices();

      // clang-format off
      return raw_devices 
         | rv::transform(extract_physical_device_information)
         | ranges::to<std::vector>();
      // clang-format on
   }

} // namespace ash::inline v0
