#pragma once

#include <vkn/core.hpp>

#include <util/containers/dynamic_array.hpp>

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#include <vulkan/vulkan.hpp>

#include <span>

namespace vkn
{
   enum struct device_error
   {
      no_physical_device_found,
      no_suitable_physical_device,
      failed_to_enumerate_queue_properties,
      no_swapchain_support,
      failed_to_create_device,
      present_queue_unavailable,
      compute_queue_unavailable,
      transfer_queue_unavailable,
      graphics_queue_unavailable,
      invalid_queue_index
   };

   auto to_string(device_error err) -> std::string;
   auto to_err_code(device_error err) -> util::error_t;

   enum struct queue_type
   {
      present,
      graphics,
      compute,
      transfer
   };

   class device
   {
   public:
      struct selection_info
      {
         vk::Instance instance{nullptr};
         vk::UniqueSurfaceKHR surface{nullptr};

         util::dynamic_array<vk::PhysicalDevice> available_devices{};

         std::uint32_t vulkan_version;

         std::shared_ptr<util::logger> p_logger;
      };

      static auto select(selection_info&& info) -> util::result<device>;

   public:
      [[nodiscard]] auto surface() const -> vk::SurfaceKHR;
      [[nodiscard]] auto logical() const -> vk::Device;
      [[nodiscard]] auto physical() const -> vk::PhysicalDevice;

      [[nodiscard]] auto vk_version() const -> std::uint32_t;

      /**
       * Get an index of a queue family that support operation related to the specified queue type.
       * If the device doesn't have any queue with the specified queue type, an error will be
       * returned.
       */
      [[nodiscard]] auto get_queue_index(queue_type type) const -> util::result<std::uint32_t>;
      /**
       * Get the index of a queue family with support for the specified queue type operations only.
       * If the device doesn't have any queue with the specified queue type, an error will be
       * returned.
       */
      [[nodiscard]] auto get_dedicated_queue_index(queue_type type) const
         -> util::result<std::uint32_t>;

      /**
       * Get a queue handle that support operation related to the specified queue type.
       * If the device doesn't have any queue with the specified queue type, an error will be
       * returned.
       */
      [[nodiscard]] auto get_queue(queue_type) const -> util::result<vk::Queue>;
      /**
       * Get a queue handle with support for the specified queue type operations only.
       * If the device doesn't have any queue with the specified queue type, an error will be
       * returned.
       */
      [[nodiscard]] [[nodiscard]] auto get_dedicated_queue(queue_type type) const
         -> util::result<vk::Queue>;

   private:
      vk::UniqueSurfaceKHR m_surface;
      vk::PhysicalDevice m_physical_device;
      vk::UniqueDevice m_logical_device;

      std::uint32_t m_version;

      std::shared_ptr<util::logger> mp_logger;
   };

   auto get_graphics_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>;

   auto get_present_queue_index(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface,
                                std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>;

   auto get_dedicated_compute_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>;

   auto get_dedicated_transfer_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>;

   auto get_separated_compute_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>;

   auto get_separated_transfer_queue_index(std::span<const vk::QueueFamilyProperties> families)
      -> monad::maybe<uint32_t>;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::device_error> : true_type
   {
   };
} // namespace std
