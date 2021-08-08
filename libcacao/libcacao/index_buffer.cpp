#include <libcacao/index_buffer.hpp>

namespace cacao
{
   auto create_buffer(const index_buffer_create_info& info) -> buffer
   {
      const mannele::u64 size = sizeof(mannele::u32) * std::size(info.indices);

      auto staging_buffer = buffer({.device = info.device,
                                    .buffer_size = size,
                                    .usage = vk::BufferUsageFlagBits::eTransferSrc,
                                    .desired_mem_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                       vk::MemoryPropertyFlagBits::eHostCoherent,
                                    .logger = info.logger});

      {
         void* p_data = info.device.logical().mapMemory(staging_buffer.memory(), 0, size, {});
         memcpy(p_data, info.indices.data(), size);
         info.device.logical().unmapMemory(staging_buffer.memory());
      }

      auto index_buffer = buffer(
         {.device = info.device,
          .buffer_size = size,
          .usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
          .desired_mem_flags = vk::MemoryPropertyFlagBits::eDeviceLocal,
          .logger = info.logger});

      const auto cmd_buffer = create_standalone_command_buffers(info.device, info.pool,
                                                                command_buffer_level::primary, 1);
      cmd_buffer[0]->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
      cmd_buffer[0]->copyBuffer(staging_buffer.value(), index_buffer.value(),
                                {vk::BufferCopy{.size = size}});
      cmd_buffer[0]->end();

      const auto queue = info.device.get_queue(queue_flag_bits::transfer).value;
      queue.submit(
         {vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &cmd_buffer[0].get()}},
         nullptr);
      queue.waitIdle();

      return index_buffer;
   }

   index_buffer::index_buffer(const index_buffer_create_info& info) :
      m_index_count(std::size(info.indices)), m_buffer(create_buffer(info)), m_logger(info.logger)
   {
      m_logger.debug("Index buffer containing {} indices ({} bytes) created", m_index_count,
                     m_index_count * sizeof(mannele::u32));
   }

   auto index_buffer::value() const noexcept -> const buffer& { return m_buffer; }
   auto index_buffer::value() noexcept -> buffer& { return m_buffer; }

   auto index_buffer::index_count() const noexcept -> mannele::u64 { return m_index_count; }
} // namespace cacao
