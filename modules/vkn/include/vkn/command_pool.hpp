/**
 * @file command_pool.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of April, 2020
 * @copyright MIT License.
 */

#pragma once

#include "vkn/core.hpp"
#include "vkn/device.hpp"

namespace vkn
{
   class command_pool
   {
      struct error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override;
         [[nodiscard]] auto message(int err) const -> std::string override;
      };

   public:
      struct create_info
      {
         vk::Device device{nullptr};
         vk::CommandPool command_pool{nullptr};

         uint32_t queue_index{0};

         util::dynamic_array<vk::CommandBuffer> primary_buffers{};
         util::dynamic_array<vk::CommandBuffer> secondary_buffers{};
      };

      enum class error_type
      {
         failed_to_create_command_pool,
         failed_to_allocate_primary_command_buffers,
         failed_to_allocate_secondary_command_buffers
      };

      command_pool(const create_info& info);
      command_pool(create_info&& info);
      command_pool(const command_pool&) = delete;
      command_pool(command_pool&& rhs) noexcept;
      ~command_pool();

      auto operator=(const command_pool&) -> command_pool& = delete;
      auto operator=(command_pool&& rhs) noexcept -> command_pool&;

      inline static auto make_error_code(error_type err) -> std::error_code
      {
         return {static_cast<int>(err), m_category};
      }

   private:
      vk::Device m_device{nullptr};
      vk::CommandPool m_command_pool{nullptr};

      uint32_t m_queue_index{0};

      util::dynamic_array<vk::CommandBuffer> m_primary_buffers;
      util::dynamic_array<vk::CommandBuffer> m_secondary_buffers;

      inline static const error_category m_category{};

   public:
      class builder
      {
      public:
         builder(const vkn::device& device, util::logger* plogger);

         auto build() noexcept -> vkn::result<command_pool>;

         auto set_queue_family_index(uint32_t index) noexcept -> builder&;

         auto set_primary_buffer_count(uint32_t count) noexcept -> builder&;
         auto set_secondary_buffer_count(uint32_t count) noexcept -> builder&;

      private:
         auto create_command_pool(vk::CommandPool handle) -> vkn::result<command_pool>;
         auto create_primary_buffers(vk::CommandPool pool)
            -> vkn::result<util::dynamic_array<vk::CommandBuffer>>;
         auto create_secondary_buffers(vk::CommandPool handle)
            -> vkn::result<util::dynamic_array<vk::CommandBuffer>>;

      private:
         util::logger* m_plogger;

         struct info
         {
            vk::Device device{nullptr};

            uint32_t queue_family_index{0};

            uint32_t primary_buffer_count{0};
            uint32_t secondary_buffer_count{0};
         } m_info;
      };
   };
} // namespace vkn
