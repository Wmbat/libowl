/**
 * @file physical_device.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of June, 2020
 * @copyright MIT License
 */

#pragma once

#include <epona_core/detail/monad/either.hpp>
#include <epona_core/graphics/vkn/instance.hpp>

namespace core::gfx::vkn
{
   namespace detail
   {
      maybe<uint32_t> get_graphics_queue_index(
         const range_over<vk::QueueFamilyProperties> auto& families)
      {
         for (uint32_t i = 0; const auto& fam : families)
         {
            if (fam.queueFlags & vk::QueueFlagBits::eGraphics)
            {
               return i;
            }

            ++i;
         }

         return monad::none;
      }

      maybe<uint32_t> get_present_queue_index(vk::PhysicalDevice physical_device,
         vk::SurfaceKHR surface, const range_over<vk::QueueFamilyProperties> auto& families)
      {
         for (uint32_t i = 0; i < families.size(); ++i)
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

      maybe<uint32_t> get_dedicated_compute_queue_index(
         const range_over<vk::QueueFamilyProperties> auto& families)
      {
         for (uint32_t i = 0; const auto& fam : families)
         {
            if ((fam.queueFlags & vk::QueueFlagBits::eCompute) &&
               (static_cast<uint32_t>(fam.queueFlags & vk::QueueFlagBits::eGraphics) == 0) &&
               (static_cast<uint32_t>(fam.queueFlags & vk::QueueFlagBits::eTransfer) == 0))
            {
               return i;
            }

            ++i;
         }

         return monad::none;
      }

      maybe<uint32_t> get_dedicated_transfer_queue_index(
         const range_over<vk::QueueFamilyProperties> auto& families)
      {
         for (uint32_t i = 0; const auto& fam : families)
         {
            if ((fam.queueFlags & vk::QueueFlagBits::eTransfer) &&
               (static_cast<uint32_t>(fam.queueFlags & vk::QueueFlagBits::eGraphics) == 0) &&
               (static_cast<uint32_t>(fam.queueFlags & vk::QueueFlagBits::eCompute) == 0))
            {
               return i;
            }

            ++i;
         }

         return monad::none;
      }

      maybe<uint32_t> get_separated_compute_queue_index(
         const range_over<vk::QueueFamilyProperties> auto& families)
      {
         maybe<uint32_t> compute{};
         for (uint32_t i = 0; const auto& fam : families)
         {
            if ((fam.queueFlags & vk::QueueFlagBits::eCompute) &&
               (static_cast<uint32_t>(fam.queueFlags & vk::QueueFlagBits::eGraphics) == 0))
            {
               if (static_cast<uint32_t>(families[i].queueFlags & vk::QueueFlagBits::eTransfer) ==
                  0)
               {
                  return i;
               }
               else
               {
                  compute = i;
               }
            }

            ++i;
         }

         return compute;
      }

      maybe<uint32_t> get_separated_transfer_queue_index(
         const range_over<vk::QueueFamilyProperties> auto& families)
      {
         maybe<uint32_t> transfer = monad::none;
         for (uint32_t i = 0; const auto& fam : families)
         {
            if ((fam.queueFlags & vk::QueueFlagBits::eTransfer) &&
               (static_cast<uint32_t>(fam.queueFlags & vk::QueueFlagBits::eGraphics) == 0))
            {
               if (static_cast<uint32_t>(fam.queueFlags & vk::QueueFlagBits::eCompute) == 0)
               {
                  return i;
               }
               else
               {
                  transfer = i;
               }
            }

            ++i;
         }

         return transfer;
      }
   } // namespace detail

   /**
    * @class physical_device <epona_core/graphics/vkn/physical_device.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Saturday, 20th of June, 2020
    * @copyright MIT License
    */
   struct physical_device
   {
      enum class type
      {
         other = 0,
         integrated = 1,
         discrete = 2,
         virtual_gpu = 3,
         cpu = 4,
      };

      enum class error
      {
         failed_to_retrieve_physical_device_count,
         failed_to_enumerate_physical_devices,
         no_physical_device_found,
         no_suitable_device
      };

      physical_device() = default;
      physical_device(const physical_device&) = delete;
      physical_device(physical_device&&);
      ~physical_device();

      physical_device& operator=(const physical_device&) = delete;
      physical_device& operator=(physical_device&&);

      bool has_dedicated_compute_queue() const;
      bool has_dedicated_transfer_queue() const;

      bool has_separated_compute_queue() const;
      bool has_separated_transfer_queue() const;

      std::string name{};

      vk::PhysicalDeviceFeatures features{};
      vk::PhysicalDeviceProperties properties{};
      vk::PhysicalDeviceMemoryProperties mem_properties{};

      vk::Instance h_instance{};

      vk::PhysicalDevice h_device{};
      vk::SurfaceKHR h_surface{};

      tiny_dynamic_array<vk::QueueFamilyProperties, 5> queue_families{};
   };

   /**
    * @class physical_device_selector <epona_core/graphics/vkn/physical_device.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Saturday, 20th of June, 2020
    * @copyright MIT License
    */
   class physical_device_selector
   {
   public:
      physical_device_selector(const instance& inst, logger* p_logger = nullptr);

      result<physical_device> select();

      physical_device_selector& set_prefered_gpu_type(physical_device::type type) noexcept;
      physical_device_selector& set_surface(vk::SurfaceKHR&& surface) noexcept;
      physical_device_selector& allow_any_gpu_type(bool allow = true) noexcept;
      physical_device_selector& require_present(bool require = true) noexcept;
      physical_device_selector& require_dedicated_compute() noexcept;
      physical_device_selector& require_dedicated_transfer() noexcept;
      physical_device_selector& require_separated_compute() noexcept;
      physical_device_selector& require_separated_transfer() noexcept;
      physical_device_selector& select_first_gpu() noexcept;

   private:
      logger* p_logger;

      struct system_info
      {
         vk::Instance instance{};
         vk::SurfaceKHR surface{};

         dynamic_array<const char*> instance_extensions;
      } sys_info;

      struct selection_info
      {
         physical_device::type prefered_type = physical_device::type::discrete;
         bool allow_any_gpu_type = true;
         bool require_present = true;
         bool require_dedicated_compute = false;
         bool require_dedicated_transfer = false;
         bool require_separated_compute = false;
         bool require_separated_transfer = false;
         bool select_first_gpu = false;
      } selection_info;

      struct physical_device_description
      {
         vk::PhysicalDevice phys_device;

         tiny_dynamic_array<vk::QueueFamilyProperties, 5> queue_families{};

         vk::PhysicalDeviceFeatures features{};
         vk::PhysicalDeviceProperties properties{};
         vk::PhysicalDeviceMemoryProperties mem_properties{};
      };

      enum class suitable
      {
         yes,
         partial,
         no
      };

   private:
      physical_device_description populate_device_details(vk::PhysicalDevice) const noexcept;

      suitable is_device_suitable(const physical_device_description& desc) const noexcept;
   };
} // namespace core::gfx::vkn

namespace std
{
   template <>
   struct is_error_code_enum<core::gfx::vkn::physical_device::error> : true_type
   {
   };
} // namespace std
