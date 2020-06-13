#include "epona_core/vk/physical_device.hpp"

#include <optional>
#include <ranges>

namespace core::vk
{
   namespace detail
   {
      option<uint32_t> get_graphics_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept
      {
         for (uint32_t i = 0; i < families.size(); ++i)
         {
            if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
               return i;
            }
         }

         return {};
      }

      option<uint32_t> get_present_queue_index(VkPhysicalDevice physical_device,
         VkSurfaceKHR surface,
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept
      {
         for (uint32_t i = 0; i < families.size(); ++i)
         {
            VkBool32 present_support = VK_FALSE;
            if (surface != VK_NULL_HANDLE)
            {
               if (vkGetPhysicalDeviceSurfaceSupportKHR(
                      physical_device, i, surface, &present_support) != VK_SUCCESS)
               {
                  return {};
               }
            }

            if (present_support == VK_TRUE)
            {
               return i;
            }
         }

         return {};
      }

      option<uint32_t> get_dedicated_compute_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept
      {
         for (uint32_t i = 0; i < families.size(); ++i)
         {
            if ((families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
               (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
               (families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0)
            {
               return i;
            }
         }

         return {};
      }

      option<uint32_t> get_dedicated_transfer_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept
      {
         for (uint32_t i = 0; i < families.size(); ++i)
         {
            if ((families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
               ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
               ((families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
            {
               return i;
            }
         }

         return {};
      }

      option<uint32_t> get_separated_compute_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept
      {
         option<uint32_t> compute{};

         for (uint32_t i = 0; i < families.size(); ++i)
         {
            if ((families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
               ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
               if ((families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0)
               {
                  return i;
               }
               else
               {
                  compute = i;
               }
            }
         }

         return compute;
      }

      option<uint32_t> get_separated_transfer_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept
      {
         option<uint32_t> transfer = {};
         for (uint32_t i = 0; i < families.size(); ++i)
         {
            if ((families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
               ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
               if ((families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)
               {
                  return i;
               }
               else
               {
                  transfer = i;
               }
            }
         }

         return transfer;
      }

      struct physical_device_error_category : std::error_category
      {
         const char* name() const noexcept override { return "vk_physical_device"; }
         std::string message(int err) const override
         {
            return physical_device::to_string(static_cast<physical_device::error>(err));
         }
      };

      const physical_device_error_category physical_device_error_cat;
   } // namespace detail

   std::string physical_device::to_string(error err)
   {
      switch (err)
      {
         case error::failed_retrieve_physical_device_count:
            return "failed_retrieve_physical_device_count";
         case error::failed_enumerate_physical_devices:
            return "failed_enumerate_physical_devices";
         case error::no_physical_device_found:
            return "no_physical_device_found";
         case error::no_suitable_device:
            return "no_suitable_device";
         default:
            return "UNKNOWN";
      }
   };

   std::error_code physical_device::make_error_code(error err)
   {
      return {static_cast<int>(err), detail::physical_device_error_cat};
   }

   physical_device::physical_device(physical_device&& other) noexcept { *this = std::move(other); }
   physical_device::~physical_device()
   {
      if (vk_instance != VK_NULL_HANDLE && vk_surface != VK_NULL_HANDLE)
      {
         vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
         vk_surface = VK_NULL_HANDLE;
      }

      vk_instance = VK_NULL_HANDLE;
      vk_physical_device = VK_NULL_HANDLE;
   }

   physical_device& physical_device::operator=(physical_device&& rhs) noexcept
   {
      if (this == &rhs)
      {
         return *this;
      }

      vk_instance = rhs.vk_instance;
      rhs.vk_instance = VK_NULL_HANDLE;

      vk_surface = rhs.vk_surface;
      rhs.vk_surface = VK_NULL_HANDLE;

      vk_physical_device = rhs.vk_physical_device;
      rhs.vk_physical_device = VK_NULL_HANDLE;

      std::swap(features, rhs.features);
      std::swap(properties, rhs.properties);
      std::swap(mem_properties, rhs.mem_properties);
      std::swap(name, rhs.name);

      queue_families = std::move(rhs.queue_families);

      return *this;
   }

   bool physical_device::has_dedicated_compute_queue() const
   {
      return detail::get_dedicated_compute_queue_index(queue_families).has_value();
   }
   bool physical_device::has_dedicated_transfer_queue() const
   {
      return detail::get_dedicated_transfer_queue_index(queue_families).has_value();
   }

   bool physical_device::has_separated_compute_queue() const
   {
      return detail::get_separated_compute_queue_index(queue_families).has_value();
   }
   bool physical_device::has_separated_transfer_queue() const
   {
      return detail::get_separated_transfer_queue_index(queue_families).has_value();
   }

   physical_device_selector::physical_device_selector(const instance& inst, logger* p_logger) :
      p_logger{p_logger}
   {
      sys_info.instance = inst.vk_instance;
      sys_info.instance_extensions = inst.extensions;
   }

   detail::result<physical_device> physical_device_selector::select()
   {
      tiny_dynamic_array<VkPhysicalDevice, 4> physical_devices;

      using error = physical_device::error;

      uint32_t device_count = 0;
      if (vkEnumeratePhysicalDevices(sys_info.instance, &device_count, nullptr) != VK_SUCCESS)
      {
         return monad::right_t<detail::error>{
            physical_device::make_error_code(error::failed_retrieve_physical_device_count)};
      }

      if (device_count == 0)
      {
         return monad::right_t<detail::error>{
            physical_device::make_error_code(error::no_physical_device_found)};
      }

      physical_devices.resize(device_count);

      if (vkEnumeratePhysicalDevices(sys_info.instance, &device_count, physical_devices.data()) !=
         VK_SUCCESS)
      {
         return monad::right_t<detail::error>{physical_device::make_error_code(
            physical_device::error::failed_enumerate_physical_devices)};
      }

      tiny_dynamic_array<physical_device_description, 4> physical_device_descriptions;
      physical_device_descriptions.reserve(physical_devices.size());

      std::ranges::for_each(physical_devices, [&](VkPhysicalDevice device) {
         physical_device_descriptions.push_back(populate_device_details(device));
      });

      physical_device_description selected{};
      if (selection_info.select_first_gpu)
      {
         selected = physical_device_descriptions[0];
      }
      else
      {
         for (const auto& desc : physical_device_descriptions)
         {
            const auto suitable = is_device_suitable(desc);
            if (suitable == suitable::yes)
            {
               selected = desc;
               break;
            }
            else if (suitable == suitable::partial)
            {
               selected = desc;
            }
         }
      }

      if (selected.phys_device == VK_NULL_HANDLE)
      {
         return monad::right_t<detail::error>{
            physical_device::make_error_code(error::no_suitable_device)};
      }

      LOG_INFO_P(p_logger, "Selected physical device: {1}", selected.properties.deviceName);

      physical_device device{};
      device.vk_instance = sys_info.instance;
      device.vk_surface = sys_info.surface;
      device.vk_physical_device = selected.phys_device;
      device.features = selected.features;
      device.properties = selected.properties;
      device.mem_properties = selected.mem_properties;
      device.queue_families = selected.queue_families;
      device.name = device.properties.deviceName;

      return monad::left_t<physical_device>{std::move(device)};
   }

   physical_device_selector& physical_device_selector::set_prefered_gpu_type(
      physical_device::type type) noexcept
   {
      selection_info.prefered_type = type;
      return *this;
   }

   physical_device_selector& physical_device_selector::set_surface(VkSurfaceKHR surface) noexcept
   {
      sys_info.surface = surface;
      return *this;
   }

   physical_device_selector& physical_device_selector::allow_any_gpu_type(bool allow) noexcept
   {
      selection_info.allow_any_gpu_type = allow;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_present(bool require) noexcept
   {
      selection_info.require_present = require;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_dedicated_compute() noexcept
   {
      selection_info.require_dedicated_compute = true;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_dedicated_transfer() noexcept
   {
      selection_info.require_dedicated_transfer = true;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_separated_compute() noexcept
   {
      selection_info.require_separated_compute = true;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_separated_transfer() noexcept
   {
      selection_info.require_separated_transfer = true;
      return *this;
   }

   physical_device_selector& physical_device_selector::select_first_gpu() noexcept
   {
      selection_info.select_first_gpu = true;
      return *this;
   }

   auto physical_device_selector::populate_device_details(VkPhysicalDevice device) const noexcept
      -> physical_device_description
   {
      physical_device_description desc{};
      desc.phys_device = device;

      uint32_t queue_count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);
      desc.queue_families.resize(queue_count);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, desc.queue_families.data());

      vkGetPhysicalDeviceFeatures(device, &desc.features);
      vkGetPhysicalDeviceProperties(device, &desc.properties);
      vkGetPhysicalDeviceMemoryProperties(device, &desc.mem_properties);

      return desc;
   };

   auto physical_device_selector::is_device_suitable(
      const physical_device_description& desc) const noexcept -> suitable
   {
      if (selection_info.require_dedicated_compute &&
         !detail::get_dedicated_compute_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (selection_info.require_dedicated_transfer &&
         !detail::get_dedicated_transfer_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (selection_info.require_separated_compute &&
         !detail::get_separated_compute_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (selection_info.require_separated_transfer &&
         !detail::get_separated_transfer_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (selection_info.require_present &&
         !detail::get_present_queue_index(desc.phys_device, sys_info.surface, desc.queue_families))
      {
         return suitable::no;
      }

      dynamic_array<VkSurfaceFormatKHR> formats;
      dynamic_array<VkPresentModeKHR> present_modes;

      {
         uint32_t count = 0;
         vkGetPhysicalDeviceSurfaceFormatsKHR(desc.phys_device, sys_info.surface, &count, nullptr);

         formats.resize(count);

         vkGetPhysicalDeviceSurfaceFormatsKHR(
            desc.phys_device, sys_info.surface, &count, formats.data());
      }

      {
         uint32_t count = 0;
         vkGetPhysicalDeviceSurfacePresentModesKHR(
            desc.phys_device, sys_info.surface, &count, nullptr);

         present_modes.resize(count);

         vkGetPhysicalDeviceSurfacePresentModesKHR(
            desc.phys_device, sys_info.surface, &count, present_modes.data());
      }

      suitable suit = suitable::no;

      if (formats.empty() || present_modes.empty())
      {
         return suitable::no;
      }

      if (desc.properties.deviceType ==
         static_cast<VkPhysicalDeviceType>(selection_info.prefered_type))
      {
         suit = suitable::yes;
      }
      else
      {
         if (selection_info.allow_any_gpu_type)
         {
            suit = suitable::partial;
         }
         else
         {
            suit = suitable::no;
         }
      }

      return suit;
   }
} // namespace core::vk
