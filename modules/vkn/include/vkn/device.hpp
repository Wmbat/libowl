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
      enum class type
      {
         present,
         graphics,
         compute,
         transfer
      };

      enum class error
      {
         present_unavailable,
         graphics_unavailable,
         compute_unavailable,
         transfer_unavailable,
         queue_index_out_of_range,
         invalid_queue_family_index
      };

      struct description
      {
         uint32_t index = 0;
         uint32_t count = 0;
         util::small_dynamic_array<float, 1> priorities;
      };
   } // namespace queue

   class device
   {
   public:
      enum class error
      {
         device_extension_not_supported,
         failed_to_create_device
      };

      struct create_info
      {
         vk::Device device{};
         uint32_t version{0u};

         util::dynamic_array<const char*> extensions{};
      };

      device() = default;
      device(physical_device physical_device, const create_info& info);
      device(physical_device physical_device, create_info&& info);
      device(const device&) = delete;
      device(device&&) noexcept;
      ~device();

      auto operator=(const device&) -> device& = delete;
      auto operator=(device&&) noexcept -> device&;

      [[nodiscard]] auto get_queue_index(queue::type type) const -> vkn::result<uint32_t>;
      [[nodiscard]] auto get_dedicated_queue_index(queue::type type) const -> vkn::result<uint32_t>;

      [[nodiscard]] auto get_queue(queue::type) const -> vkn::result<vk::Queue>;
      [[nodiscard]] [[nodiscard]] auto get_dedicated_queue(queue::type type) const
         -> vkn::result<vk::Queue>;

      [[nodiscard]] auto value() const noexcept -> const vk::Device&;
      [[nodiscard]] auto physical() const noexcept -> const physical_device&;
      [[nodiscard]] auto get_vulkan_version() const noexcept -> uint32_t;

   private:
      vkn::physical_device m_physical_device;

      vk::Device m_device;

      uint32_t m_version{0};

      util::dynamic_array<const char*> m_extensions;

   public:
      class builder
      {
      public:
         builder(const loader& vk_loader, physical_device&& phys_device, uint32_t version,
            util::logger* const plogger = nullptr);

         [[nodiscard]] auto build() -> vkn::result<device>;

         auto set_queue_setup(const util::dynamic_array<queue::description>& descriptions)
            -> builder&;
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
