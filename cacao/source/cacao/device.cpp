#include <cacao/device.hpp>

#include <cacao/error.hpp>
#include <cacao/util/assert.hpp>
#include <cacao/util/error.hpp>

#include <monads/try.hpp>

#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <map>

namespace vi = ranges::views;

namespace cacao
{
   auto to_string(const queue_flags& value) -> std::string
   {
      if (value == queue_flag_bits::none)
      {
         return "{none}";
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

      return "{" + result.substr(0, result.size() - 3) + "}";
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

   device::device(device_create_info&& info) :
      m_physical{find_physical_device(info)},
      m_logical{create_logical_device(info)},
      m_vk_version{info.ctx.vulkan_version()},
      m_queues{create_queues(info)}
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(m_logical.get());

      const auto physical_properties = m_physical.getProperties();

      util::logger_wrapper logger = info.logger;
      logger.info("[cacao] Physical device {} used", physical_properties.deviceName);

      for (const auto& queue : m_queues)
      {
         logger.info("[cacao] Device queue supporting {} created.", to_string(queue.type));
      }
   }

   auto device::logical() const -> vk::Device { return m_logical.get(); }
   auto device::physical() const -> vk::PhysicalDevice { return m_physical; }

   auto device::vk_version() const -> std::uint32_t { return m_vk_version; }

   auto device::get_queue_index(const queue_flags& type) const -> std::uint32_t
   {
      const auto queue_index = get_dedicated_queue_index(type, m_queues).or_else([&] {
         return get_specialized_queue_index(type, queue_flag_bits::graphics, m_queues).or_else([&] {
            return ::cacao::get_queue_index(type, m_queues);
         });
      });

      if (queue_index)
      {
         if (const queue* q = ranges::find(m_queues, queue_index.value(), &queue::family_index))
         {
            return q->family_index;
         }
      }

      throw runtime_error{to_error_cond(error_code::queue_type_not_found),
                          fmt::format("{} with type {}",
                                      to_string(error_code::queue_type_not_found),
                                      to_string(type))}; // implement error code
   }

   auto device::get_queue(const queue_flags& type) const -> const queue&
   {
      const auto queue_index = get_dedicated_queue_index(type, m_queues).or_else([&] {
         return get_specialized_queue_index(type, queue_flag_bits::graphics, m_queues).or_else([&] {
            return ::cacao::get_queue_index(type, m_queues);
         });
      });

      if (queue_index)
      {
         if (const queue* q = ranges::find(m_queues, queue_index.value(), &queue::family_index))
         {
            return *q;
         }
      }

      throw runtime_error{to_error_cond(error_code::queue_type_not_found),
                          fmt::format("{} with type {}",
                                      to_string(error_code::queue_type_not_found),
                                      to_string(type))}; // implement error code
   }

   auto device::find_physical_device(const device_create_info& info) const -> vk::PhysicalDevice
   {
      std::multimap<std::int32_t, vk::PhysicalDevice> candidates;

      for (vk::PhysicalDevice device : info.ctx.enumerate_physical_devices())
      {
         candidates.insert({info.physical_device_rating_fun(device), device});
      }

      if (candidates.empty())
      {
         throw runtime_error{to_error_cond(error_code::no_available_physical_device)};
      }

      if (candidates.rbegin()->first <= 0)
      {
         throw runtime_error{to_error_cond(error_code::no_suitable_physical_device)};
      }

      return candidates.rbegin()->second;
   }
   auto device::create_logical_device(const device_create_info& info) const -> vk::UniqueDevice
   {
      util::logger_wrapper logger = info.logger;

      const auto features = m_physical.getFeatures();

      // clang-format off
      const crl::dynamic_array queue_create_infos = get_queue_create_infos(info);
      const crl::dynamic_array vk_queue_create_infos = queue_create_infos 
         | vi::transform([](const detail::queue_info& info) {
               return vk::DeviceQueueCreateInfo{
                  .queueFamilyIndex = info.family,
                  .queueCount = static_cast<std::uint32_t>(std::size(info.priorities)),
                  .pQueuePriorities = std::data(info.priorities)};
           }) 
         | ranges::to<crl::dynamic_array>;
      // clang-format on

      crl::small_dynamic_array<const char*, 1> extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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
         logger.info("[cacao] Device extension: {0}", name);
      }

      return m_physical.createDeviceUnique(
         {.queueCreateInfoCount = static_cast<std::uint32_t>(std::size(vk_queue_create_infos)),
          .pQueueCreateInfos = std::data(vk_queue_create_infos),
          .enabledExtensionCount = static_cast<std::uint32_t>(std::size(extensions)),
          .ppEnabledExtensionNames = std::data(extensions),
          .pEnabledFeatures = &features});
   }
   auto device::create_queues(const device_create_info& info) const
      -> crl::small_dynamic_array<queue, 2>
   {
      crl::small_dynamic_array<queue, 2> queues;

      for (auto& queue_info : get_queue_create_infos(info))
      {
         queues.append({.value = m_logical->getQueue(queue_info.family, 0),
                        .type = queue_info.flag_bits,
                        .family_index = queue_info.family});
      }

      return queues;
   }

   auto device::get_queue_create_infos(const device_create_info& info) const
      -> const crl::dynamic_array<detail::queue_info>
   {
      monad::maybe<detail::queue_info> render_queue_properties = monad::none;

      std::vector properties = m_physical.getQueueFamilyProperties();
      std::uint32_t index = 0;
      for (auto& property : properties)
      {
         bool has_graphics =
            (property.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
         bool has_present = false;

         if (info.surface)
         {
            vk::Bool32 present_support = VK_FALSE;
            if (m_physical.getSurfaceSupportKHR(static_cast<std::uint32_t>(index), info.surface,
                                                &present_support) == vk::Result::eSuccess)
            {
               has_present = present_support == VK_TRUE;
            }
         }

         if (has_graphics && has_present)
         {
            render_queue_properties =
               detail::queue_info{.flag_bits = queue_flag_bits::present |
                                     detail::to_queue_flag_bits(property.queueFlags),
                                  .family = static_cast<std::uint32_t>(index),
                                  .priorities = {1.0f}};
         }

         index++;
      }

      crl::dynamic_array<detail::queue_info> queue_infos{};

      if (render_queue_properties)
      {
         queue_infos.append(render_queue_properties.value());
      }

      return queue_infos;
   }

   auto get_dedicated_queue_index(const queue_flags& desired, std::span<const queue> queues)
      -> monad::maybe<std::uint32_t>
   {
      for (const auto& queue : queues)
      {
         if ((queue.type & desired) == desired && (queue.type & ~desired) == queue_flag_bits::none)
         {
            return queue.family_index;
         }
      }

      return monad::none;
   }
   auto get_specialized_queue_index(const queue_flags& desired, const queue_flags& unwanted,
                                    std::span<const queue> queues) -> monad::maybe<std::uint32_t>
   {
      for (const auto& queue : queues)
      {
         if ((queue.type & desired) == desired && (queue.type & unwanted) == queue_flag_bits::none)
         {
            return queue.family_index;
         }
      }

      return monad::none;
   }
   auto get_queue_index(const queue_flags& desired, std::span<const queue> queues)
      -> monad::maybe<std::uint32_t>
   {
      for (const auto& queue : queues)
      {
         if ((queue.type & desired) == desired)
         {
            return queue.family_index;
         }
      }

      return monad::none;
   }
} // namespace cacao
