#include <libcacao/vertex_buffer.hpp>

namespace cacao
{
   auto create_buffer(const vertex_buffer_create_info& info) -> buffer
   {
      const mannele::u64 size = sizeof(vertex) * std::size(info.vertices);

      auto staging_buffer = cacao::buffer({.device = info.device,
                                    .buffer_size = size,
                                    .usage = vk::BufferUsageFlagBits::eTransferSrc,
                                    .desired_mem_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                       vk::MemoryPropertyFlagBits::eHostCoherent,
                                    .logger = info.logger});

      {
         void* p_data = info.device.logical().mapMemory(staging_buffer.memory(), 0, size, {});
         memcpy(p_data, info.vertices.data(), size);
         info.device.logical().unmapMemory(staging_buffer.memory());
      }

      auto vertex_buffer = cacao::buffer(
         {.device = info.device,
          .buffer_size = size,
          .usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
          .desired_mem_flags = vk::MemoryPropertyFlagBits::eDeviceLocal,
          .logger = info.logger});

      const auto cmd_buffer = create_standalone_command_buffers(info.device, info.pool,
                                                                command_buffer_level::primary, 1);
      cmd_buffer[0]->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
      cmd_buffer[0]->copyBuffer(staging_buffer.value(), vertex_buffer.value(),
                                {vk::BufferCopy{.size = size}});
      cmd_buffer[0]->end();

      const auto queue = info.device.get_queue(queue_flag_bits::transfer).value;
      queue.submit(
         {vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &cmd_buffer[0].get()}},
         nullptr);
      queue.waitIdle();

      return vertex_buffer;
   }

   vertex_buffer::vertex_buffer(const vertex_buffer_create_info& info) :
      m_vertex_count(std::size(info.vertices)), m_buffer(create_buffer(info)), m_logger(info.logger)
   {
      m_logger.debug("Vertex buffer containing {} vertices ({} bytes) created", m_vertex_count,
                     m_vertex_count * sizeof(vertex));
   }

   auto vertex_buffer::buffer() const noexcept -> const cacao::buffer& { return m_buffer; }
   auto vertex_buffer::buffer() noexcept -> cacao::buffer& { return m_buffer; }

   auto vertex_buffer::vertex_count() const noexcept -> mannele::u64 { return m_vertex_count; }
} // namespace cacao
