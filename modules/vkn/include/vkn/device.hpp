/**
 * @file device.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 23rd of June, 2020
 * @copyright MIT License
 */

#pragma once

#include <vkn/core.hpp>
#include <vkn/physical_device.hpp>

#include <util/containers/dynamic_array.hpp>

namespace vkn
{
   namespace queue
   {
      /**
       * The different types of supported queue types.
       */
      enum class type
      {
         present,
         graphics,
         compute,
         transfer
      };

      /**
       * The possible errors related to the creation of queues
       */
      enum class error
      {
         present_unavailable,
         graphics_unavailable,
         compute_unavailable,
         transfer_unavailable,
         queue_index_out_of_range,
         invalid_queue_family_index
      };

      /**
       * The information that describes a queue family.
       */
      struct description
      {
         uint32_t index = 0;
         uint32_t count = 0;
         util::small_dynamic_array<float, 1> priorities;
      };
   } // namespace queue

   /**
    * Holds all functionality around the lifetime and use of a vulkan device, it hold the physical
    * and logical representation of the graphics card.
    */
   class device final
   {
   public:
      /**
       * The possible error types related to the creation of the device.
       */
      enum class error
      {
         device_extension_not_supported,
         failed_to_create_device
      };

      /**
       * The information needed to construct a device instance
       */
      struct create_info
      {
         vk::Device device{};
         uint32_t version{0u};

         util::dynamic_array<const char*> extensions{};
      };

      device() = default;
      device(physical_device&& physical_device, const create_info& info);
      device(physical_device&& physical_device, create_info&& info);
      device(const device&) = delete;
      device(device&&) noexcept;
      ~device();

      auto operator=(const device&) -> device& = delete;
      auto operator=(device&&) noexcept -> device&;

      /**
       * Get an index of a queue family that support operation related to the specified queue type.
       * If the device doesn't have any queue with the specified queue type, an error will be
       * returned.
       */
      [[nodiscard]] auto get_queue_index(queue::type type) const -> vkn::result<uint32_t>;
      /**
       * Get the index of a queue family with support for the specified queue type operations only.
       * If the device doesn't have any queue with the specified queue type, an error will be
       * returned.
       */
      [[nodiscard]] auto get_dedicated_queue_index(queue::type type) const -> vkn::result<uint32_t>;

      /**
       * Get a queue handle that support operation related to the specified queue type.
       * If the device doesn't have any queue with the specified queue type, an error will be
       * returned.
       */
      [[nodiscard]] auto get_queue(queue::type) const -> vkn::result<vk::Queue>;
      /**
       * Get a queue handle with support for the specified queue type operations only.
       * If the device doesn't have any queue with the specified queue type, an error will be
       * returned.
       */
      [[nodiscard]] [[nodiscard]] auto get_dedicated_queue(queue::type type) const
         -> vkn::result<vk::Queue>;

      auto operator->() -> vk::Device*;
      auto operator->() const noexcept -> const vk::Device*;

      [[nodiscard]] auto value() const noexcept -> vk::Device;
      /**
       * Get a const reference to the physical representation of the device.
       */
      [[nodiscard]] auto physical() const noexcept -> const physical_device&;
      /**
       * Get the version of the current vulkan session.
       */
      [[nodiscard]] auto get_vulkan_version() const noexcept -> uint32_t;

   private:
      vk::Device m_device{nullptr};
      vkn::physical_device m_physical_device;

      uint32_t m_version{0};

      util::dynamic_array<const char*> m_extensions;

   public:
      /**
       * A class used to facilitate the building of a device instance.
       */
      class builder
      {
      public:
         builder(const loader& vk_loader, physical_device&& phys_device, uint32_t version,
                 util::logger* const plogger = nullptr);

         /**
          * Finalize the construction of a device object. If the construction results in a failure,
          * an error will be returned
          */
         [[nodiscard]] auto build() -> vkn::result<device>;

         /**
          * Set the descriptions of all queue families.
          */
         auto set_queue_setup(const util::dynamic_array<queue::description>& descriptions)
            -> builder&;
         /**
          * Add a device extension that should be enabled with the device construction
          */
         auto add_desired_extension(const std::string& extension_name) -> builder&;

      private:
         const loader& m_loader;

         util::logger* const m_plogger;

         struct info
         {
            physical_device phys_device;

            uint32_t api_version{};

            util::dynamic_array<queue::description> queue_descriptions;
            util::dynamic_array<const char*> desired_extensions;
         } m_info;
      };
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::device::error> : true_type
   {
   };
} // namespace std
