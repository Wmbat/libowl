#include <libash/detail/physical_device_rating.hpp>

#include <libash/queue.hpp>

#include <libreglisse/maybe.hpp>
#include <libreglisse/operations/and_then.hpp>
#include <libreglisse/operations/or_else.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/algorithm/count.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/transform.hpp>

// #include <ranges>

using reglisse::and_then;
using reglisse::maybe;
using reglisse::none;
using reglisse::or_else;
using reglisse::some;

namespace stdr = std::ranges;
namespace rv = ranges::views;
namespace ra = ranges::actions;

namespace ash::inline v0
{
   namespace
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
      static constexpr i32 compute_queue_support_value = 10;
      static constexpr i32 transfer_queue_support_value = 10;

      /**
       * @brief Helper function to extract the extensionName field of a vk::ExtensionProperties
       */
      auto to_extension_name(const vk::ExtensionProperties& property) -> std::string_view
      {
         return property.extensionName;
      }

      /**
       * @brief Convert a C string into a string_view
       */
      auto to_string_view(const char* const str) -> const std::string_view { return str; }

      /**
       * @brief Check if all required extensions are supported by the physical device
       *
       * @param[in] device_exts The range of all extension names supported by the physical device
       * @param[in] required_ext_names The range of extension names required by the user
       *
       * @return True if all extensions required by the user are supported by the physical device,
       * otherwise false
       */
      auto are_all_required_ext_supported(std::ranges::range auto device_exts,
                                          std::ranges::range auto required_ext_names) -> bool
      {
         // clang-format off
         auto supported = required_ext_names 
            | rv::filter([=](std::string_view name) {
               return ranges::find(device_exts, name) != std::end(device_exts);
            });
         // clang-format on

         const auto supported_count = std::ranges::distance(supported);
         const auto required_count = static_cast<i32>(std::size(required_ext_names));

         return supported_count == required_count;
      }

      /**
       * @brief
       *
       * @param[in] properties The physical device properties containing the available api version
       * @param[in] minimum_version The minimum allowed vulkan api version
       * @param[in] desired_version The prefered available vulkan api version
       *
       * @return A scored based on whether the prefered or minimum api version is met. If the
       * physical device version doesn't meet the minimum, return -1.
       */
      auto rate_property_api_version(const vk::PhysicalDeviceProperties& properties,
                                     const mannele::semantic_version& minimum_version,
                                     const mannele::semantic_version& desired_version) -> i32
      {
         const auto device_version =
            mannele::semantic_version{.major = VK_VERSION_MAJOR(properties.apiVersion),
                                      .minor = VK_VERSION_MINOR(properties.apiVersion),
                                      .patch = VK_VERSION_PATCH(properties.apiVersion)};

         if (minimum_version > device_version)
         {
            return -1;
         }
         else if (desired_version >= device_version)
         {
            return preferred_api_version_value;
         }
         else
         {
            return minimum_api_version_value;
         }
      }
      /**
       * @brief
       *
       * @param[in]
       * @param[in]
       *
       * @return
       */
      auto rate_property_device_type(const vk::PhysicalDeviceProperties& properties,
                                     physical_device_type prefered_type) -> i32
      {
         i32 rating = 0;

         if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
         {
            const i32 preliminary = prefered_type == physical_device_type::discrete
                                       ? prefered_physical_device_type_value
                                       : discrete_physical_device_type_value;

            rating += preliminary;
         }
         else if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
         {
            rating += prefered_type == physical_device_type::integrated
                         ? prefered_physical_device_type_value
                         : integrated_physical_device_type_value;
         }
         else if (properties.deviceType == vk::PhysicalDeviceType::eVirtualGpu)
         {
            rating += prefered_type == physical_device_type::virtual_gpu
                         ? prefered_physical_device_type_value
                         : virtual_physical_device_type_value;
         }
         else if (properties.deviceType == vk::PhysicalDeviceType::eCpu)
         {
            rating += prefered_type == physical_device_type::cpu
                         ? prefered_physical_device_type_value
                         : cpu_physical_device_type_value;
         }
         else
         {
            rating += prefered_type == physical_device_type::other
                         ? prefered_physical_device_type_value
                         : other_physical_device_type_value;
         }

         return rating;
      }

      /**
       * @brief Helper struct used to hold relevant queue family data for optimal physical device
       * queue selection
       */
      struct queue_family_info
      {
         vk::QueueFlags flag; //< The operations supported by the queue family
         u32 family;          //< The index of the queue in the physical device
         u32 count;           //< The number of queues available from this family
      };

      /**
       * @brief
       *
       * @param[in] p
       *
       * @return
       */
      auto to_queue_info(const std::pair<u32, const vk::QueueFamilyProperties>& p)
         -> queue_family_info
      {
         return {.flag = p.second.queueFlags, .family = p.first, .count = p.second.queueCount};
      }

      /**
       * @brief Check if queue family supports a specific set of queue operations
       *
       * @param[in] q The queue family
       * @param[in] type The operations to check for support
       *
       * @return True if the operations are supported by the queue family, otherwise false
       */
      auto does_queue_support_type(const queue_family_info& q, const vk::QueueFlags& type) -> bool
      {
         return (q.flag & type) == type;
      }
      /**
       * @brief Check if a queue family supports graphics operations
       *
       * @param[in] q The queue family
       *
       * @return True if the queue family support graphics operations, otherwise false
       */
      auto does_queue_support_graphics(const queue_family_info& q) -> bool
      {
         return does_queue_support_type(q, vk::QueueFlagBits::eGraphics);
      }
      /**
       * @brief Check if a queue family supports transfer operations
       *
       * @param[in] q The queue family
       *
       * @return True if the queue family support transfer operations, otherwise false
       */
      auto does_queue_support_transfer(const queue_family_info& q) -> bool
      {
         return does_queue_support_type(q, vk::QueueFlagBits::eTransfer);
      }
      /**
       * @brief Check if a queue family supports compute operations
       *
       * @param[in] q The queue family
       *
       * @return True if the queue family support compute operations, otherwise false
       */
      auto does_queue_support_compute(const queue_family_info& q) -> bool
      {
         return does_queue_support_type(q, vk::QueueFlagBits::eCompute);
      }
      /**
       * @brief Check if a queue family supports all (graphics, transfer and compute) operations
       *
       * @param[in] q The queue family
       *
       * @return True if the queue family support all operations, otherwise false
       */
      auto does_queue_support_all_ops(const queue_family_info& q) -> bool
      {
         return does_queue_support_graphics(q) and does_queue_support_compute(q)
                and does_queue_support_transfer(q);
      }

      /**
       * @brief Check if a queue family is dedicated to a specific queue operation
       *
       * @param[in] q The queue family
       * @param[in] type The operation the queue should have.
       * @param[in] negators The operations the queue should not support
       *
       * @return True if q only support type
       */
      auto is_queue_dedicated_to_type(const queue_family_info& q, const vk::QueueFlags& type,
                                      auto... negators) -> bool
      {
         return does_queue_support_type(q, type)
                and (... and !does_queue_support_type(q, negators));
      }
      /**
       * @brief Check if a queue family has support for only transfer operations
       *
       * @param[in] q The queue family
       *
       * @return True if and only if the queue family only supports transfer operations, otherwise
       * false
       */
      auto is_queue_dedicated_to_transfer(const queue_family_info& q) -> bool
      {
         return is_queue_dedicated_to_type(q, vk::QueueFlagBits::eTransfer,
                                           vk::QueueFlagBits::eCompute,
                                           vk::QueueFlagBits::eGraphics);
      }
      /**
       * @brief Check if a queue family has support for only compute operations
       *
       * @param[in] q The queue family
       *
       * @return True if and only if the queue family only supports compute operations, otherwise
       * false
       */
      auto is_queue_dedicated_to_compute(const queue_family_info& q) -> bool
      {
         return is_queue_dedicated_to_type(q, vk::QueueFlagBits::eCompute,
                                           vk::QueueFlagBits::eTransfer,
                                           vk::QueueFlagBits::eGraphics);
      }

      /**
       * @brief
       *
       * @param[in]
       * @param[in]
       *
       * @return
       */
      auto adjust_desired_queue_index(const queue_family_info& input,
                                      std::span<const desired_queue_data> already_selected)
         -> maybe<desired_queue_data>
      {
         const auto flags = input.flag;
         const u32 family = input.family;
         const u32 index = ranges::count(already_selected, family, &desired_queue_data::family);

         if (index <= input.count)
         {
            return some(desired_queue_data{.flags = flags, .family = family, .index = index});
         }
         else
         {
            return none;
         }
      }

      /**
       * @brief
       *
       * @param[in]
       *
       * @return
       */
      auto find_best_general_queue(std::span<const queue_family_info> infos)
         -> maybe<queue_family_info>
      {
         auto general_queues = infos | rv::filter(does_queue_support_all_ops);
         const auto it = stdr::max_element(general_queues, {}, &queue_family_info::count);

         if (it != std::ranges::end(general_queues))
         {
            return some(*it);
         }
         else
         {
            return none;
         }
      }

      /**
       * @brief Find the best queue family that only supports transfer operations
       *
       * @param[in] available_queues A contiguous range of available queue family info from the
       * physical device
       *
       * @return Either a queue_family_info struct if a queue family is dedicated to transfer, or
       * nothing
       */
      auto find_best_dedicated_transfer_queue(std::span<const queue_family_info> available_queues)
         -> maybe<queue_family_info>
      {
         auto transfer_queues = available_queues | rv::filter(is_queue_dedicated_to_transfer);
         const auto it = stdr::max_element(transfer_queues, {}, &queue_family_info::count);

         if (it != stdr::end(transfer_queues))
         {
            return some(*it);
         }
         else
         {
            return none;
         }
      }
      /**
       * @brief Find the best queue family that only supports transfer operations
       *
       * @param[in] available_queues A contiguous range of available queue family info from the
       * physical device
       *
       * @return Either a queue_family_info struct if a queue family is dedicated to transfer, or
       * nothing
       */
      auto find_best_separated_transfer_queue(std::span<const queue_family_info> available_queues)
         -> maybe<queue_family_info>
      {
         // clang-format off
         auto transfer_queues = available_queues 
            | rv::filter(does_queue_support_transfer)
            | rv::remove_if(does_queue_support_compute);
         const auto it = stdr::max_element(transfer_queues, {}, &queue_family_info::count);
         // clang-format on

         if (it != stdr::end(transfer_queues))
         {
            return some(*it);
         }
         else
         {
            return none;
         }
      }

      /**
       * @brief Find the best queue family that supports transfer but doesn't support graphics
       * operations.
       *
       * @param[in] available_queues
       * @param[in] selected_queues
       *
       * @return
       */
      auto find_best_suited_transfer_queue(std::span<const queue_family_info> available_queues,
                                           std::span<const desired_queue_data> selected_queues)
         -> maybe<desired_queue_data>
      {
         // clang-format off
         return find_best_dedicated_transfer_queue(available_queues) 
            | or_else([=] {
                 return find_best_separated_transfer_queue(available_queues);
              })
            | or_else([=] {
                 return find_best_general_queue(available_queues);
              })
            | and_then([=](queue_family_info&& info) {
                 return adjust_desired_queue_index(info, selected_queues);
              });
         // clang-format on
      }

      /**
       * @brief Find the best queue family that only supports compute operations
       *
       * @param[in] available_queues A contiguous range of available queue family info from the
       * physical device
       *
       * @return Either a queue_family_info struct if a queue family is dedicated to compute, or
       * nothing
       */
      auto find_best_dedicated_compute_queue(std::span<const queue_family_info> available_queues)
         -> maybe<queue_family_info>
      {
         auto transfer_queues = available_queues | rv::filter(is_queue_dedicated_to_compute);
         const auto it = stdr::max_element(transfer_queues, {}, &queue_family_info::count);

         if (it != stdr::end(transfer_queues))
         {
            return some(*it);
         }
         else
         {
            return none;
         }
      }
      auto find_best_separated_compute_queue(std::span<const queue_family_info> available_queues)
         -> maybe<queue_family_info>
      {
         // clang-format off
         auto transfer_queues = available_queues 
            | rv::filter(does_queue_support_compute)
            | rv::remove_if(does_queue_support_compute);
         const auto it = stdr::max_element(transfer_queues, {}, &queue_family_info::count);
         // clang-format on

         if (it != stdr::end(transfer_queues))
         {
            return some(*it);
         }
         else
         {
            return none;
         }
      }

      /**
       * @brief
       *
       * @param[in] available_queues
       * @param[in] selected_queues
       *
       * @return
       */
      auto find_best_suited_compute_queue(std::span<const queue_family_info> available_queues,
                                          std::span<const desired_queue_data> selected_queues)
         -> maybe<desired_queue_data>
      {
         // clang-format off
         return find_best_dedicated_compute_queue(available_queues) 
            | or_else([=] {
                 return find_best_separated_compute_queue(available_queues);
              })
            | or_else([=] {
                 return find_best_general_queue(available_queues);
              })
            | and_then([=](queue_family_info&& info) {
                 return adjust_desired_queue_index(info, selected_queues);
              });
         // clang-format on
      }

      /**
       * @brief
       *
       * @param[in] available_queues A range of queue families sorted by the number of queues
       * available in each family, largest families at the front
       */
      auto find_all_necessary_queues(std::span<const queue_family_info> available_queues,
                                     bool require_transfer, bool require_compute)
         -> std::pair<std::vector<desired_queue_data>, i32>
      {
         std::vector<desired_queue_data> selected_queues;
         selected_queues.reserve(3);

         if (maybe q =
                find_best_general_queue(available_queues) | and_then([&](queue_family_info&& info) {
                   return adjust_desired_queue_index(info, selected_queues);
                }))
         {
            q.borrow().flags = vk::QueueFlagBits::eGraphics;
            selected_queues.push_back(q.borrow());
         }
         else
         {
            return {{}, -1};
         }

         if (maybe q = find_best_suited_transfer_queue(available_queues, selected_queues))
         {
            q.borrow().flags = vk::QueueFlagBits::eTransfer;
            selected_queues.push_back(q.borrow());
         }
         else
         {
            if (require_transfer)
            {
               return {{}, -1};
            }
         }

         if (maybe q = find_best_suited_compute_queue(available_queues, selected_queues))
         {
            q.borrow().flags = vk::QueueFlagBits::eCompute;
            selected_queues.push_back(q.borrow());
         }
         else
         {
            if (require_compute)
            {
               return {{}, -1};
            }
         }

         return {selected_queues,
                 detail::tally_ratings(compute_queue_support_value, transfer_queue_support_value)};
      }

   } // namespace

   namespace detail
   {
      auto rate_properties_support(const vk::PhysicalDeviceProperties& properties,
                                   const physical_device_select_info& info) -> i32
      {
         return tally_ratings(
            rate_property_api_version(properties, info.minimum_version, info.desired_version),
            rate_property_device_type(properties, info.prefered_type));
      }

      auto rate_extension_support(std::span<const vk::ExtensionProperties> all_device_exts,
                                  const physical_device_select_info& info)
         -> std::pair<std::vector<std::string_view>, i32>
      {
         const auto required_extensions = info.required_extensions | rv::transform(to_string_view);
         const auto desired_extensions = info.desired_extensions | rv::transform(to_string_view);
         const auto device_ext_names = all_device_exts | rv::transform(to_extension_name);

         if (!are_all_required_ext_supported(device_ext_names, required_extensions))
         {
            return {{}, -1};
         }

         const auto supported_exts =
            rv::concat(required_extensions,
                       desired_extensions | rv::filter([=](std::string_view name) {
                          return ranges::find(device_ext_names, name) != std::end(device_ext_names);
                       }))
            | ranges::to<std::vector>;

         const auto rating = std::size(supported_exts) * supported_extension_value;

         return {supported_exts, rating};
      }

      auto rate_queue_support(std::span<const vk::QueueFamilyProperties> queue_properties,
                              const physical_device_select_info& info)
         -> std::pair<std::vector<desired_queue_data>, i32>
      {
         // clang-format off
         const auto queue_infos = queue_properties 
            | rv::enumerate 
            | rv::transform(to_queue_info)
            | ranges::to<std::vector>
            | ra::sort(ranges::greater{}, &queue_family_info::count);
         // clang-format on

         return find_all_necessary_queues(queue_infos, info.require_transfer_queue,
                                          info.require_compute_queue);
      }
   } // namespace detail
} // namespace ash::inline v0
