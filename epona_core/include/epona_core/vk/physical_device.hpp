#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/details/monad/option.hpp"
#include "epona_core/vk/detail/includes.hpp"
#include "epona_core/vk/instance.hpp"
#include <type_traits>

namespace core::vk
{
   namespace detail
   {
      option<uint32_t> get_graphics_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept;

      option<uint32_t> get_present_queue_index(VkPhysicalDevice physical_device,
         VkSurfaceKHR surface,
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept;

      option<uint32_t> get_dedicated_compute_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept;

      option<uint32_t> get_dedicated_transfer_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept;

      option<uint32_t> get_separated_compute_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept;

      option<uint32_t> get_separated_transfer_queue_index(
         const tiny_dynamic_array<VkQueueFamilyProperties, 5>& families) noexcept;
   } // namespace detail

   struct physical_device
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
         failed_retrieve_physical_device_count,
         failed_enumerate_physical_devices,
         no_physical_device_found,
         no_suitable_device
      };

   public:
      physical_device() = default;
      physical_device(const physical_device& other) = delete;
      physical_device(physical_device&& other) noexcept;
      ~physical_device();

      physical_device& operator=(const physical_device& rhs) = delete;
      physical_device& operator=(physical_device&& rhs) noexcept;

      bool has_dedicated_compute_queue() const;
      bool has_dedicated_transfer_queue() const;

      bool has_separated_compute_queue() const;
      bool has_separated_transfer_queue() const;

   public:
      static std::string to_string(error err);
      static std::error_code make_error_code(error err);

   public:
      std::string name{};

      VkInstance vk_instance{VK_NULL_HANDLE};
      VkPhysicalDevice vk_physical_device{VK_NULL_HANDLE};
      VkSurfaceKHR vk_surface{VK_NULL_HANDLE};

      VkPhysicalDeviceFeatures features{};
      VkPhysicalDeviceProperties properties{};
      VkPhysicalDeviceMemoryProperties mem_properties{};

      tiny_dynamic_array<VkQueueFamilyProperties, 5> queue_families{};
   };

   class physical_device_selector
   {
   public:
      physical_device_selector(const instance& inst, logger* p_logger = nullptr);

      detail::result<physical_device> select();

      physical_device_selector& set_prefered_gpu_type(physical_device::type type) noexcept;
      physical_device_selector& set_surface(VkSurfaceKHR surface) noexcept;
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
         VkInstance instance = VK_NULL_HANDLE;
         VkSurfaceKHR surface = VK_NULL_HANDLE;

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
         VkPhysicalDevice phys_device = VK_NULL_HANDLE;

         tiny_dynamic_array<VkQueueFamilyProperties, 5> queue_families{};

         VkPhysicalDeviceFeatures features{};
         VkPhysicalDeviceProperties properties{};
         VkPhysicalDeviceMemoryProperties mem_properties{};
      };

      enum class suitable
      {
         yes,
         partial,
         no
      };

   private:
      physical_device_description populate_device_details(VkPhysicalDevice device) const noexcept;

      suitable is_device_suitable(const physical_device_description& desc) const noexcept;
   };
} // namespace core::vk

namespace std
{
   template <>
   struct is_error_code_enum<core::vk::physical_device::error> : true_type
   {
   };
} // namespace std
