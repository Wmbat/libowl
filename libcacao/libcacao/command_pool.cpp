/**
 * @file libcacao/command_pool.cpp
 * @author wmbat wmbat@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#include <libcacao/command_pool.hpp>

// Third Party Libraries

#include <libreglisse/try.hpp>

// C++ Standard Library

#include <cassert>
#include <string>
#include <vector>

namespace cacao
{
   class command_pool_error_category : public std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "cacao_command_pool";
      }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return std::string(magic_enum::enum_name(static_cast<command_pool_error>(err)));
      }
   };

   inline static const command_pool_error_category command_pool_category;

   auto make_error_condition(command_pool_error code) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(code), command_pool_category});
   }

   auto allocate_buffers(vk::Device device, vk::CommandPool pool, vk::CommandBufferLevel level,
                         mannele::u32 count) -> std::vector<vk::CommandBuffer>
   {
      if (count > 0)
      {
         return device.allocateCommandBuffers(
            {.commandPool = pool, .level = level, .commandBufferCount = count});
      }

      return {};
   }

   command_pool::command_pool(const command_pool_create_info& info) :
      m_queue_index(
         info.queue_family_index
            ? info.queue_family_index.borrow()
            : info.device.find_best_suited_queue(queue_flag_bits::graphics).family_index),
      m_pool(info.device.logical().createCommandPoolUnique({.queueFamilyIndex = m_queue_index})),
      m_primary_buffers(allocate_buffers(info.device.logical(), m_pool.get(),
                                         vk::CommandBufferLevel::ePrimary,
                                         info.primary_buffer_count)),
      m_secondary_buffers(allocate_buffers(info.device.logical(), m_pool.get(),
                                           vk::CommandBufferLevel::eSecondary,
                                           info.secondary_buffer_count)),
      m_logger(info.logger)
   {
      m_logger.debug(
         "Command pool created on queue family {} with {} primary buffers and {} secondary buffers",
         m_queue_index, info.primary_buffer_count, info.secondary_buffer_count);
   }
   command_pool::command_pool(command_pool_create_info&& info) :
      m_queue_index(
         info.queue_family_index
            ? info.queue_family_index.borrow()
            : info.device.find_best_suited_queue(queue_flag_bits::graphics).family_index),
      m_pool(info.device.logical().createCommandPoolUnique({.queueFamilyIndex = m_queue_index})),
      m_primary_buffers(allocate_buffers(info.device.logical(), m_pool.get(),
                                         vk::CommandBufferLevel::ePrimary,
                                         info.primary_buffer_count)),
      m_secondary_buffers(allocate_buffers(info.device.logical(), m_pool.get(),
                                           vk::CommandBufferLevel::eSecondary,
                                           info.secondary_buffer_count)),
      m_logger(info.logger)
   {
      m_logger.debug(
         "Command pool created on queue family {} with {} primary buffers and {} secondary buffers",
         m_queue_index, info.primary_buffer_count, info.secondary_buffer_count);
   }

   auto command_pool::value() const noexcept -> vk::CommandPool { return m_pool.get(); }
   auto command_pool::primary_buffers() const noexcept -> std::span<const vk::CommandBuffer>
   {
      return m_primary_buffers;
   }
   auto command_pool::secondary_buffers() const noexcept -> std::span<const vk::CommandBuffer>
   {
      return m_secondary_buffers;
   }

   auto to_vk_level(command_buffer_level level)
   {
      return level == command_buffer_level::primary ? vk::CommandBufferLevel::ePrimary
                                                    : vk::CommandBufferLevel::eSecondary;
   }

   auto LIBCACAO_SYMEXPORT create_standalone_command_buffers(const device& device,
                                                             const command_pool& pool,
                                                             command_buffer_level level,
                                                             mannele::u32 count)
      -> std::vector<vk::UniqueCommandBuffer>
   {
      assert(count != 0); // NOLINT

      return device.logical().allocateCommandBuffersUnique(
         {.commandPool = pool.value(), .level = to_vk_level(level), .commandBufferCount = count});
   }
} // namespace cacao
