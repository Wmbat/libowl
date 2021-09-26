/**
 * @file libcacao/device.cpp
 * @author wmbat wmbat@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#include <libcacao/device.hpp>

#include <libcacao/error.hpp>

// Third Party Libraries

#include <libreglisse/operations/or_else.hpp>
#include <libreglisse/try.hpp>

#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>

// C++ Standard Library

#include <algorithm>
#include <map>
#include <vector>

namespace vi = ranges::views;

using reglisse::maybe;
using reglisse::none;
using reglisse::or_else;
using reglisse::some;

using mannele::u32;

namespace cacao
{
   auto to_string(const queue_flags& value) -> std::string
   {
      if (value == queue_flag_bits::none)
      {
         return "{ none }";
      }

      std::string result;

      if (value & queue_flag_bits::graphics)
         result += "graphics | ";
      if (value & queue_flag_bits::present)
         result += "present | ";
      if (value & queue_flag_bits::compute)
         result += "compute | ";
      if (value & queue_flag_bits::transfer)
         result += "transfer | ";

      return "{ " + result.substr(0, result.size() - 3) + " }";
   }

   namespace detail
   {
      auto to_queue_flag_bits(const vk::QueueFlags& flags) -> queue_flags
      {
         queue_flags bits = queue_flag_bits::none;

         if ((flags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
         {
            bits |= queue_flag_bits::graphics;
         }

         if ((flags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
         {
            bits |= queue_flag_bits::compute;
         }

         if ((flags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer)
         {
            bits |= queue_flag_bits::transfer;
         }

         return bits;
      }

      struct queue_info
      {
         queue_flags flag_bits;
         mannele::u32 family;
         mannele::u32 count;
         std::vector<float> priorities;
      };
   } // namespace detail

   auto rate_physical_device(vk::PhysicalDevice device) -> std::int32_t
   {
      const auto properties = device.getProperties();

      if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
      {
         return 1000; // NOLINT
      }
      if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
      {
         return 500; // NOLINT
      }

      return -1;
   }

   auto handle_extra_queue_family(vk::Device device, const detail::queue_info& dedicated,
                                  const detail::queue_info& general, queue_flag_bits extra_queue)
      -> std::vector<queue>;

   auto get_queue_create_infos(vk::PhysicalDevice physical, vk::SurfaceKHR surface)
      -> const std::vector<detail::queue_info>;

   device::device(device_create_info&& info) :
      m_physical{find_physical_device(info)}, m_logical{create_logical_device(info)},
      m_vk_version{info.ctx.vulkan_version()}, m_queues{create_queues(info)}
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(m_logical.get());

      const auto physical_properties = m_physical.getProperties();

      mannele::log_ptr logger = info.logger;
      logger.info("GPU: {}", physical_properties.deviceName);

      for (const auto& queue : m_queues)
      {
         logger.debug("Device queue from family {} supporting {} created.", queue.family_index,
                      to_string(queue.type));
      }
   }

   auto device::logical() const -> vk::Device { return m_logical.get(); }
   auto device::physical() const -> vk::PhysicalDevice { return m_physical; }

   auto device::vk_version() const -> std::uint32_t { return m_vk_version; }

   auto device::find_physical_device(const device_create_info& info) const -> vk::PhysicalDevice
   {
      std::multimap<std::int32_t, vk::PhysicalDevice> candidates;

      for (vk::PhysicalDevice device : info.ctx.enumerate_physical_devices())
      {
         candidates.insert({info.physical_device_rating_fun(device), device});
      }

      if (candidates.empty())
      {
         throw runtime_error{to_error_condition(error_code::no_available_physical_device)};
      }

      if (candidates.rbegin()->first <= 0)
      {
         throw runtime_error{to_error_condition(error_code::no_suitable_physical_device)};
      }

      return candidates.rbegin()->second;
   }
   auto device::create_logical_device(const device_create_info& info) const -> vk::UniqueDevice
   {
      mannele::log_ptr logger = info.logger;

      const auto features = m_physical.getFeatures();

      // clang-format off
      const std::vector queue_create_infos = get_queue_create_infos(m_physical, info.surface);
      const std::vector vk_queue_create_infos = queue_create_infos 
         | vi::transform([](const detail::queue_info& info) {
               return vk::DeviceQueueCreateInfo{
                  .queueFamilyIndex = info.family,
                  .queueCount = static_cast<std::uint32_t>(std::size(info.priorities)),
                  .pQueuePriorities = std::data(info.priorities)};
           }) 
         | ranges::to<std::vector>;
      // clang-format on

      std::vector<const char*> extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

      for (const auto& desired : extensions)
      {
         bool is_present = false;
         for (const auto& available : m_physical.enumerateDeviceExtensionProperties())
         {
            if (strcmp(desired, static_cast<const char*>(available.extensionName)) == 0)
            {
               is_present = true;
            }
         }

         if (!is_present)
         {
            // error
         }
      }

      for (const char* name : extensions)
      {
         logger.debug("Device extension: {0}", name);
      }

      return m_physical.createDeviceUnique(
         {.queueCreateInfoCount = static_cast<std::uint32_t>(std::size(vk_queue_create_infos)),
          .pQueueCreateInfos = std::data(vk_queue_create_infos),
          .enabledExtensionCount = static_cast<std::uint32_t>(std::size(extensions)),
          .ppEnabledExtensionNames = std::data(extensions),
          .pEnabledFeatures = &features});
   }
   auto device::create_queues(const device_create_info& info) const -> std::vector<queue>
   {
      using detail::queue_info;

      std::vector<queue> queues;

      std::vector queue_infos = get_queue_create_infos(m_physical, info.surface);

      /**
       * We have a separate queue for Compute, Transfer and General
       */
      if (std::size(queue_infos) == 3)
      {
         for (const auto& queue_info : queue_infos)
         {
            queues.push_back(queue{.value = m_logical->getQueue(queue_info.family, 0),
                                   .type = queue_info.flag_bits,
                                   .family_index = queue_info.family});
         }
      }
      /**
       * We know either transfer or compute is from generar purpose
       */
      else if (std::size(queue_infos) == 2)
      {
         const auto& family_one = queue_infos[0];
         const auto& family_two = queue_infos[1];

         if (family_one.flag_bits == queue_flag_bits::transfer)
         {
            queues = handle_extra_queue_family(m_logical.get(), family_one, family_two,
                                               queue_flag_bits::compute);
         }
         else if (family_two.flag_bits == queue_flag_bits::transfer)
         {
            queues = handle_extra_queue_family(m_logical.get(), family_two, family_one,
                                               queue_flag_bits::compute);
         }
         else if (family_one.flag_bits == queue_flag_bits::compute)
         {
            queues = handle_extra_queue_family(m_logical.get(), family_one, family_two,
                                               queue_flag_bits::transfer);
         }
         else if (family_two.flag_bits == queue_flag_bits::compute)
         {
            queues = handle_extra_queue_family(m_logical.get(), family_two, family_one,
                                               queue_flag_bits::transfer);
         }
      }
      /**
       * We only have 1 queue family
       */
      else
      {
         const auto& queue_info = queue_infos[0];

         queues.push_back(queue{.value = m_logical->getQueue(queue_info.family, 0),
                                .type = queue_info.flag_bits,
                                .family_index = queue_info.family});

         if (queue_info.count > 1)
         {
            queues.push_back(queue{.value = m_logical->getQueue(queue_info.family, 1),
                                   .type = queue_flag_bits::transfer,
                                   .family_index = queue_info.family});
         }

         if (queue_info.count > 2)
         {
            queues.push_back(queue{.value = m_logical->getQueue(queue_info.family, 2),
                                   .type = queue_flag_bits::compute,
                                   .family_index = queue_info.family});
         }
      }

      ranges::sort(queues, {}, &queue::family_index);

      return queues;
   }

   auto find_dedicated_queue(std::span<const queue> queues, const queue_flags& desired)
      -> maybe<queue>;
   auto find_separated_queue(std::span<const queue> queues, const queue_flags& desired,
                             const queue_flags& unwanted) -> maybe<queue>;
   auto find_any_queue(std::span<const queue> queues, const queue_flags& desired) -> maybe<queue>;

   auto device::find_best_suited_queue(const queue_flags& desired) const -> queue
   {
      // clang-format off
      
      const queue_flags unwanted = queue_flag_bits::graphics;

      return (find_dedicated_queue(m_queues, desired) 
            | or_else([&] { return find_separated_queue(m_queues, desired, unwanted); }) 
            | or_else([&] { return find_any_queue(m_queues, desired); })
         ).take();

      // clang-format on
   }

   auto find_dedicated_queue(std::span<const queue> queues, const queue_flags& desired)
      -> maybe<queue>
   {
      for (const queue& q : queues)
      {
         if ((q.type & desired) == desired and (q.type & ~desired) == queue_flag_bits::none)
         {
            return some(q);
         }
      }

      return none;
   }
   auto find_separated_queue(std::span<const queue> queues, const queue_flags& desired,
                             const queue_flags& unwanted) -> maybe<queue>
   {
      for (const queue& q : queues)
      {
         if ((q.type & desired) == desired && (q.type & unwanted) == queue_flag_bits::none)
         {
            return some(q);
         }
      }

      return none;
   }
   auto find_any_queue(std::span<const queue> queues, const queue_flags& desired) -> maybe<queue>
   {
      for (const queue& q : queues)
      {
         if ((q.type & desired) == desired)
         {
            return some(q);
         }
      }

      return none;
   }

   auto handle_extra_queue_family(vk::Device device, const detail::queue_info& dedicated,
                                  const detail::queue_info& general, queue_flag_bits extra_queue)
      -> std::vector<queue>
   {
      std::vector<queue> queues;

      queues.push_back(queue{.value = device.getQueue(dedicated.family, 0),
                             .type = dedicated.flag_bits,
                             .family_index = dedicated.family});

      queues.push_back(queue{.value = device.getQueue(general.family, 0),
                             .type = general.flag_bits,
                             .family_index = general.family});

      if (general.count > 1)
      {
         queues.push_back(queue{.value = device.getQueue(general.family, 1),
                                .type = extra_queue,
                                .family_index = general.family});
      }

      return queues;
   }

   auto does_queue_support_present(vk::PhysicalDevice physical, vk::SurfaceKHR surface, u32 index)
      -> bool
   {
      if (surface)
      {
         vk::Bool32 present_support = VK_FALSE;
         if (physical.getSurfaceSupportKHR(index, surface, &present_support) ==
             vk::Result::eSuccess)
         {
            return present_support == VK_TRUE;
         }
      }

      return false;
   }
   auto does_queue_support_graphics(vk::QueueFlags flags) -> bool
   {
      return (flags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
   }
   auto does_queue_support_transfer(vk::QueueFlags flags) -> bool
   {
      return (flags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;
   }
   auto does_queue_support_compute(vk::QueueFlags flags) -> bool
   {
      return (flags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute;
   }

   auto to_queue_info(ranges::common_pair<int, const vk::QueueFamilyProperties&> p)
      -> detail::queue_info
   {
      return {.flag_bits =
                 queue_flag_bits::present | detail::to_queue_flag_bits(p.second.queueFlags),
              .family = static_cast<u32>(p.first),
              .count = p.second.queueCount,
              .priorities = std::vector(p.second.queueCount, 1.0f)};
   }

   auto find_separated_transfer(std::span<const vk::QueueFamilyProperties> properties)
      -> maybe<detail::queue_info>
   {
      for (const auto [index, property] : vi::enumerate(properties))
      {
         const bool has_graphics = does_queue_support_graphics(property.queueFlags);
         const bool has_transfer = does_queue_support_transfer(property.queueFlags);
         const bool has_compute = does_queue_support_compute(property.queueFlags);

         if (!has_graphics and !has_compute and has_transfer)
         {
            return some(detail::queue_info{.flag_bits = queue_flag_bits::transfer,
                                           .family = static_cast<u32>(index),
                                           .count = 1,
                                           .priorities = {1.0f}});
         }
      }

      for (const auto [index, property] : vi::enumerate(properties))
      {
         const bool has_graphics = does_queue_support_graphics(property.queueFlags);
         const bool has_transfer = does_queue_support_transfer(property.queueFlags);

         if (!has_graphics and has_transfer)
         {
            return some(detail::queue_info{.flag_bits = queue_flag_bits::transfer,
                                           .family = static_cast<u32>(index),
                                           .count = 1,
                                           .priorities = {1.0f}});
         }
      }

      return none;
   }
   auto find_separated_compute(std::span<const vk::QueueFamilyProperties> properties)
      -> maybe<detail::queue_info>
   {
      for (const auto [index, property] : vi::enumerate(properties))
      {
         const bool has_graphics = does_queue_support_graphics(property.queueFlags);
         const bool has_transfer = does_queue_support_transfer(property.queueFlags);
         const bool has_compute = does_queue_support_compute(property.queueFlags);

         if (not has_graphics and not has_transfer and has_compute)
         {
            return some(detail::queue_info{.flag_bits = queue_flag_bits::compute,
                                           .family = static_cast<u32>(index),
                                           .count = 1,
                                           .priorities = {1.0f}});
         }
      }

      for (const auto [index, property] : vi::enumerate(properties))
      {
         const bool has_graphics = does_queue_support_graphics(property.queueFlags);
         const bool has_compute = does_queue_support_compute(property.queueFlags);

         if (not has_graphics and has_compute)
         {
            return some(detail::queue_info{.flag_bits = queue_flag_bits::compute,
                                           .family = static_cast<u32>(index),
                                           .count = 1,
                                           .priorities = {1.0f}});
         }
      }

      return none;
   }
   auto find_general_purpose(vk::PhysicalDevice physical, vk::SurfaceKHR surface,
                             std::span<const vk::QueueFamilyProperties> properties)
      -> maybe<detail::queue_info>
   {
      using detail::queue_info;
      using queue_properties_t = vk::QueueFamilyProperties;
      using pair_t = ranges::common_pair<int, const queue_properties_t&>;

      const auto is_general_purpose_property = [&](pair_t p) {
         const u32 index = static_cast<u32>(p.first);
         const bool has_present = does_queue_support_present(physical, surface, index);
         const bool has_graphics = does_queue_support_graphics(p.second.queueFlags);
         const bool has_transfer = does_queue_support_transfer(p.second.queueFlags);
         const bool has_compute = does_queue_support_compute(p.second.queueFlags);

         return has_present and has_graphics and has_transfer and has_compute;
      };

      // clang-format off
      auto curated_properties = properties 
         | vi::enumerate 
         | vi::filter(is_general_purpose_property) 
         | vi::transform(to_queue_info);
      // clang-format on

      const auto it = ranges::max_element(curated_properties, {}, &queue_info::count);
      if (it != std::end(curated_properties))
      {
         return some(*it);
      }

      return none;
   }

   auto get_queue_create_infos(vk::PhysicalDevice physical, vk::SurfaceKHR surface)
      -> const std::vector<detail::queue_info>
   {
      using detail::queue_info;

      std::vector properties = physical.getQueueFamilyProperties();

      maybe dedicated_transfer_queue = find_separated_transfer(properties);
      maybe dedicated_compute_queue = find_separated_compute(properties);
      maybe general_purpose_queue = find_general_purpose(physical, surface, properties);

      std::vector<detail::queue_info> queue_infos{};

      if (general_purpose_queue.is_some())
      {
         const auto& main_queue = general_purpose_queue.borrow();

         u32 desired_queue_count = 1;
         if (dedicated_transfer_queue.is_some())
         {
            queue_infos.push_back(dedicated_transfer_queue.borrow());
         }
         else
         {
            ++desired_queue_count;
         }

         if (dedicated_compute_queue.is_some())
         {
            queue_infos.push_back(dedicated_compute_queue.borrow());
         }
         else
         {
            ++desired_queue_count;
         }

         queue_infos.push_back(queue_info{.flag_bits = main_queue.flag_bits,
                                          .family = main_queue.family,
                                          .count = desired_queue_count,
                                          .priorities = std::vector(desired_queue_count, 1.0f)});
      }

      return queue_infos;
   }
} // namespace cacao
