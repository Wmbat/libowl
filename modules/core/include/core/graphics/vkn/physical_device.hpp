/**
 * @file physical_device.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of June, 2020
 * @copyright MIT License
 */

#pragma once

#include <core/graphics/vkn/instance.hpp>

#include <util/monad/either.hpp>

namespace core::gfx::vkn
{
   namespace detail
   {
      auto get_graphics_queue_index(
         const util::range_over<vk::QueueFamilyProperties> auto& families) -> util::maybe<uint32_t>
      {
         for (uint32_t i = 0; const auto& fam : families)
         {
            if (fam.queueFlags & vk::QueueFlagBits::eGraphics)
            {
               return i;
            }

            ++i;
         }

         return util::to_maybe();
      }

      auto get_present_queue_index(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface,
         const util::range_over<vk::QueueFamilyProperties> auto& families) -> util::maybe<uint32_t>
      {
         for (uint32_t i = 0; i < families.size(); ++i)
         {
            VkBool32 present_support = VK_FALSE;
            if (surface)
            {
               if (physical_device.getSurfaceSupportKHR(i, surface, &present_support) !=
                  vk::Result::eSuccess)
               {
                  return util::to_maybe();
               }
            }

            if (present_support == VK_TRUE)
            {
               return i;
            }
         }

         return util::to_maybe();
      }

      auto get_dedicated_compute_queue_index(
         const util::range_over<vk::QueueFamilyProperties> auto& families) -> util::maybe<uint32_t>
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

         return util::to_maybe();
      }

      auto get_dedicated_transfer_queue_index(
         const util::range_over<vk::QueueFamilyProperties> auto& families) -> util::maybe<uint32_t>
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

         return util::to_maybe();
      }

      auto get_separated_compute_queue_index(
         const util::range_over<vk::QueueFamilyProperties> auto& families) -> util::maybe<uint32_t>
      {
         util::maybe<uint32_t> compute{};
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

      auto get_separated_transfer_queue_index(
         const util::range_over<vk::QueueFamilyProperties> auto& families) -> util::maybe<uint32_t>
      {
         util::maybe<uint32_t> transfer = util::to_maybe();
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
   class physical_device
   {
   public:
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

      struct create_info
      {
         std::string_view name{};
         vk::PhysicalDeviceFeatures features{};
         vk::PhysicalDeviceProperties properties{};
         vk::PhysicalDeviceMemoryProperties mem_properties{};
         vk::Instance instance{nullptr};
         vk::PhysicalDevice device{nullptr};
         vk::SurfaceKHR surface{nullptr};
         util::dynamic_array<vk::QueueFamilyProperties> queue_families{};
      };

   public:
      physical_device() = default;
      physical_device(const create_info& info);
      physical_device(const physical_device&) = delete;
      physical_device(physical_device&&) noexcept;
      ~physical_device();

      auto operator=(const physical_device&) -> physical_device& = delete;
      auto operator=(physical_device&&) noexcept -> physical_device&;

      [[nodiscard]] auto has_dedicated_compute_queue() const -> bool;
      [[nodiscard]] auto has_dedicated_transfer_queue() const -> bool;

      [[nodiscard]] auto has_separated_compute_queue() const -> bool;
      [[nodiscard]] auto has_separated_transfer_queue() const -> bool;

      [[nodiscard]] auto value() const noexcept -> const vk::PhysicalDevice&;
      [[nodiscard]] auto features() const noexcept -> const vk::PhysicalDeviceFeatures&;
      [[nodiscard]] auto surface() const noexcept -> const vk::SurfaceKHR&;
      [[nodiscard]] auto queue_families() const
         -> const util::dynamic_array<vk::QueueFamilyProperties>;

   private:
      std::string m_name{};

      vk::PhysicalDeviceFeatures m_features{};
      vk::PhysicalDeviceProperties m_properties{};
      vk::PhysicalDeviceMemoryProperties m_mem_properties{};

      vk::Instance m_instance{nullptr};
      vk::PhysicalDevice m_device{nullptr};
      vk::SurfaceKHR m_surface{nullptr};

      util::dynamic_array<vk::QueueFamilyProperties> m_queue_families{};

   public:
      class selector
      {
      public:
         selector(const instance& instance, util::logger* plogger = nullptr);

         [[nodiscard]] auto select() -> vkn::result<physical_device>;

         auto set_prefered_gpu_type(physical_device::type type) noexcept -> selector&;
         auto set_surface(vk::SurfaceKHR&& surface) noexcept -> selector&;
         auto allow_any_gpu_type(bool allow = true) noexcept -> selector&;
         auto require_present(bool require = true) noexcept -> selector&;
         auto require_dedicated_compute() noexcept -> selector&;
         auto require_dedicated_transfer() noexcept -> selector&;
         auto require_separated_compute() noexcept -> selector&;
         auto require_separated_transfer() noexcept -> selector&;
         auto select_first_gpu() noexcept -> selector&;

      private:
         util::logger* m_plogger;

         struct system_info
         {
            vk::Instance instance{};
            vk::SurfaceKHR surface{};

            util::dynamic_array<const char*> instance_extensions;
         } m_system_info;

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
         } m_selection_info;

         struct physical_device_description
         {
            vk::PhysicalDevice phys_device;

            util::tiny_dynamic_array<vk::QueueFamilyProperties, 16> queue_families{};

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
         [[nodiscard]] auto populate_device_details(vk::PhysicalDevice) const
            -> physical_device_description;

         [[nodiscard]] auto is_device_suitable(const physical_device_description& desc) const
            -> suitable;
      };
   };
} // namespace core::gfx::vkn

namespace std
{
   template <>
   struct is_error_code_enum<core::gfx::vkn::physical_device::error> : true_type
   {
   };
} // namespace std
