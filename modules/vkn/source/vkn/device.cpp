#include <vkn/device.hpp>

#include <monads/try.hpp>

#include <range/v3/view.hpp>

#include <map>
#include <span>

namespace vkn
{
   struct queue_description
   {
      std::uint32_t index = 0;
      std::uint32_t count = 0;
      util::small_dynamic_array<float, 1> priorities;
   };

   struct device_data
   {
      vk::UniqueSurfaceKHR surface;
      vk::PhysicalDevice physical_device;
      vk::UniqueDevice logical_device;

      std::uint32_t version;

      util::logger_wrapper logger;
   };

   auto find_suitable_device(std::span<const vk::PhysicalDevice> devices, device_data&& data)
      -> util::result<device_data>;
   auto create_device(device_data&& data) -> util::result<device_data>;

   auto device::select(selection_info&& info) -> util::result<device>
   {
      const auto finalize = [&](device_data&& data) {
         device dev{};
         dev.m_surface = std::move(data.surface);
         dev.m_physical_device = data.physical_device;
         dev.m_logical_device = std::move(data.logical_device);
         dev.m_logger = data.logger;

         VULKAN_HPP_DEFAULT_DISPATCHER.init(dev.m_logical_device.get());

         return dev;
      };

      return find_suitable_device(info.available_devices,
                                  {.surface = std::move(info.surface),
                                   .version = info.vulkan_version,
                                   .logger = info.logger})
         .and_then(create_device)
         .map(finalize);
   }

   auto device::surface() const -> vk::SurfaceKHR { return m_surface.get(); }
   auto device::logical() const -> vk::Device { return m_logical_device.get(); }
   auto device::physical() const -> vk::PhysicalDevice { return m_physical_device; }

   auto device::vk_version() const -> std::uint32_t { return m_version; }

   auto device::get_queue_index(queue_type type) const -> util::result<std::uint32_t>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_physical_device.getQueueFamilyProperties();
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(device_error::failed_to_enumerate_queue_properties);
         })
         .and_then(
            [&](std::vector<vk::QueueFamilyProperties>&& families) -> util::result<std::uint32_t> {
               if (type == queue_type::present)
               {
                  const auto index =
                     get_present_queue_index(m_physical_device, m_surface.get(), families);

                  if (!index)
                  {
                     return monad::err(to_err_code(device_error::present_queue_unavailable));
                  }

                  return index.value_or(0u);
               }
               if (type == queue_type::graphics)
               {
                  if (auto i = get_graphics_queue_index(families))
                  {
                     return i.value();
                  }

                  return monad::err(to_err_code(device_error::graphics_queue_unavailable));
               }
               if (type == queue_type::compute)
               {
                  if (const auto i = get_separated_compute_queue_index(families))
                  {
                     return i.value();
                  }

                  return monad::err(to_err_code(device_error::compute_queue_unavailable));
               }
               if (type == queue_type::transfer)
               {
                  if (const auto i = get_separated_transfer_queue_index(families))
                  {
                     return i.value();
                  }

                  return monad::err(to_err_code(device_error::transfer_queue_unavailable));
               }

               return monad::err(to_err_code(device_error::invalid_queue_index));
            });
   }

   auto device::get_dedicated_queue_index(queue_type type) const -> util::result<uint32_t>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_physical_device.getQueueFamilyProperties();
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(device_error::failed_to_enumerate_queue_properties);
         })
         .and_then(
            [&](std::vector<vk::QueueFamilyProperties>&& families) -> util::result<std::uint32_t> {
               if (type == queue_type::compute)
               {
                  if (auto i = get_dedicated_compute_queue_index(families))
                  {
                     return i.value();
                  }

                  return monad::err(to_err_code(device_error::compute_queue_unavailable));
               }
               if (type == queue_type::transfer)
               {
                  if (auto i = get_dedicated_transfer_queue_index(families))
                  {
                     return i.value();
                  }

                  return monad::err(to_err_code(device_error::transfer_queue_unavailable));
               }

               return monad::err(to_err_code(device_error::invalid_queue_index));
            });
   }

   auto device::get_queue(queue_type type) const -> util::result<vk::Queue>
   {
      return get_queue_index(type).map([&](std::uint32_t i) {
         return m_logical_device->getQueue(i, 0);
      });
   }

   auto device::get_dedicated_queue([[maybe_unused]] queue_type type) const
      -> util::result<vk::Queue>
   {
      return get_dedicated_queue_index(type).map([&](std::uint32_t i) {
         return m_logical_device->getQueue(i, 0);
      });
   }

   auto is_device_suitable(vk::PhysicalDevice device, vk::SurfaceKHR surface) -> bool
   {
      auto format_res = monad::try_wrap<vk::SystemError>([&] {
                           return device.getSurfaceFormatsKHR(surface);
                        }).map_error([]([[maybe_unused]] auto err) {
         return false;
      });

      if (auto err = format_res.error())
      {
         return err.value();
      }

      auto mode_res = monad::try_wrap<vk::SystemError>([&] {
                         return device.getSurfacePresentModesKHR(surface);
                      }).map_error([]([[maybe_unused]] auto err) {
         return false;
      });

      if (auto err = mode_res.error())
      {
         return err.value();
      }

      const auto formats = format_res.value().value();
      const auto present_modes = mode_res.value().value();

      if (formats.empty() || present_modes.empty())
      {
         return false;
      }

      return true;
   }

   auto rate_physical_device(vk::PhysicalDevice device, vk::SurfaceKHR surface) -> std::int32_t
   {
      if (is_device_suitable(device, surface))
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

      return -1;
   }

   auto find_suitable_device(std::span<const vk::PhysicalDevice> devices, device_data&& data)
      -> util::result<device_data>
   {
      std::multimap<std::int32_t, vk::PhysicalDevice> candidates;

      for (vk::PhysicalDevice device : devices)
      {
         candidates.insert({rate_physical_device(device, data.surface.get()), device});
      }

      if (candidates.empty())
      {
         return monad::err(to_err_code(device_error::no_physical_device_found));
      }

      if (candidates.rbegin()->first <= 0)
      {
         return monad::err(to_err_code(device_error::no_suitable_physical_device));
      }

      data.physical_device = candidates.rbegin()->second;

      auto properties = data.physical_device.getProperties();

      data.logger.info("[vulkan] physical device selected: {}", properties.deviceName);

      return std::move(data);
   }

   auto get_queue_descriptions(vk::PhysicalDevice device)
      -> util::result<util::dynamic_array<queue_description>>
   {
      // clang-format off
      return monad::try_wrap<vk::SystemError>([&] {
                return device.getQueueFamilyProperties();
             })
         .map([]([[maybe_unused]] auto&& properties) {
            return ranges::views::iota(0U, std::size(properties)) 
               | ranges::views::transform([](std::size_t i) {
                      return queue_description{.index = static_cast<std::uint32_t>(i),
                                               .count = 1U,
                                               .priorities =
                                                  util::small_dynamic_array<float, 1>{1.0f}};
                   }) 
               | ranges::to<util::dynamic_array>;
         })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(device_error::failed_to_enumerate_queue_properties);
         });
      // clang-format on
   }

   auto generate_queue_infos(std::span<const queue_description> description)
      -> util::dynamic_array<vk::DeviceQueueCreateInfo>
   {
      // clang-format off
      return description 
         | ranges::views::transform([](const queue_description& desc) {
               return vk::DeviceQueueCreateInfo{.queueFamilyIndex = desc.index,
                                                .queueCount = desc.count,
                                                .pQueuePriorities = std::data(desc.priorities)};
            }) 
         | ranges::to<util::dynamic_array>;
      // clang-format on
   }

   auto create_device(device_data&& data) -> util::result<device_data>
   {
      const auto description_res = get_queue_descriptions(data.physical_device);
      const auto queue_create_info_res = description_res.map(generate_queue_infos);

      if (auto err = queue_create_info_res.error())
      {
         return monad::err(err.value());
      }

      util::dynamic_array<const char*> extensions{};
      if (data.surface)
      {
         extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      }

      for (const auto& desired : extensions)
      {
         bool is_present = false;
         for (const auto& available : data.physical_device.enumerateDeviceExtensionProperties())
         {
            if (strcmp(desired, static_cast<const char*>(available.extensionName)) == 0)
            {
               is_present = true;
            }
         }

         if (!is_present)
         {
            monad::err(to_err_code(device_error::no_swapchain_support));
         }
      }

      for (const char* name : extensions)
      {
         data.logger.info("[vulkan] device extension: {0}", name);
      }

      const auto queue_create_infos = queue_create_info_res.value().value();
      const auto features = data.physical_device.getFeatures();

      return monad::try_wrap<vk::SystemError>([&] {
                return data.physical_device.createDeviceUnique(
                   {.queueCreateInfoCount =
                       static_cast<std::uint32_t>(std::size(queue_create_infos)),
                    .pQueueCreateInfos = std::data(queue_create_infos),
                    .enabledExtensionCount = static_cast<std::uint32_t>(std::size(extensions)),
                    .ppEnabledExtensionNames = std::data(extensions),
                    .pEnabledFeatures = &features});
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(device_error::failed_to_create_device);
         })
         .map([&](vk::UniqueDevice device) {
            data.logical_device = std::move(device);

            data.logger.info("[vulkan] logical device created");

            return std::move(data);
         });
   }

   auto get_graphics_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>
   {
      for (std::size_t i : ranges::views::iota(0U, families.size()))
      {
         if (families[i].queueFlags & vk::QueueFlagBits::eGraphics)
         {
            return i;
         }
      }

      return monad::none;
   }

   auto get_present_queue_index(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface,
                                std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>
   {
      for (uint32_t i : ranges::views::iota(0U, families.size()))
      {
         VkBool32 present_support = VK_FALSE;
         if (surface)
         {
            if (physical_device.getSurfaceSupportKHR(i, surface, &present_support) !=
                vk::Result::eSuccess)
            {
               return monad::none;
            }
         }

         if (present_support == VK_TRUE)
         {
            return i;
         }
      }

      return monad::none;
   }

   auto get_dedicated_compute_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>
   {
      for (uint32_t i : ranges::views::iota(0U, families.size()))
      {
         if ((families[i].queueFlags & vk::QueueFlagBits::eCompute) &&
             (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eGraphics) == 0) &&
             (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eTransfer) == 0))
         {
            return i;
         }
      }

      return monad::none;
   }

   auto get_dedicated_transfer_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>
   {
      for (uint32_t i : ranges::views::iota(0U, families.size()))
      {
         if ((families[i].queueFlags & vk::QueueFlagBits::eTransfer) &&
             (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eGraphics) == 0) &&
             (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eCompute) == 0))
         {
            return i;
         }
      }

      return monad::none;
   }

   auto get_separated_compute_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>
   {
      monad::maybe<uint32_t> compute{};

      for (uint32_t i : ranges::views::iota(0U, families.size()))
      {
         if ((families[i].queueFlags & vk::QueueFlagBits::eCompute) &&
             (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eGraphics) == 0))
         {
            if (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eTransfer) == 0)
            {
               return i;
            }

            compute = i;
         }
      }

      return compute;
   }

   auto get_separated_transfer_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>
   {
      monad::maybe<uint32_t> transfer{};

      for (uint32_t i : ranges::views::iota(0U, families.size()))
      {
         if ((families[i].queueFlags & vk::QueueFlagBits::eTransfer) &&
             (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eGraphics) == 0))
         {
            if (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eCompute) == 0)
            {
               return i;
            }

            transfer = i;
         }
      }

      return transfer;
   }

   struct device_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override { return "vulkan_device"; }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<device_error>(err));
      }
   };

   static const device_error_category device_error_cat{};

   auto to_string(device_error err) -> std::string
   {
      if (err == device_error::no_suitable_physical_device)
      {
         return "no_suitable_physical_device";
      }
      if (err == device_error::no_physical_device_found)
      {
         return "no_physical_device_found";
      }
      if (err == device_error::failed_to_enumerate_queue_properties)
      {
         return "failed_to_enumerate_queue_properties";
      }
      if (err == device_error::no_swapchain_support)
      {
         return "no_swapchain_support";
      }
      if (err == device_error::failed_to_create_device)
      {
         return "failed_to_create_device";
      }
      if (err == device_error::present_queue_unavailable)
      {
         return "present_queue_unavailable";
      }
      if (err == device_error::compute_queue_unavailable)
      {
         return "compute_queue_unavailable";
      }
      if (err == device_error::transfer_queue_unavailable)
      {
         return "transfer_queue_unavailable";
      }
      if (err == device_error::graphics_queue_unavailable)
      {
         return "graphics_queue_unavailable";
      }
      if (err == device_error::invalid_queue_index)
      {
         return "invalid_queue_index";
      }

      return "UNKNOWN";
   }
   auto to_err_code(device_error err) -> util::error_t
   {
      return {{static_cast<int>(err), device_error_cat}};
   }
} // namespace vkn
