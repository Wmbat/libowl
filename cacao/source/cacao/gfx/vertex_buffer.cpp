#include <cacao/gfx/vertex_buffer.hpp>

namespace cacao
{
   auto to_string(vertex_buffer_error err) -> std::string
   {
      switch (err)
      {
         case vertex_buffer_error::failed_to_create_staging_buffer:
            return "failed_to_create_staging_buffer";
         case vertex_buffer_error::failed_to_create_vertex_buffer:
            return "failed_to_create_vertex_buffer";
         case vertex_buffer_error::failed_to_create_command_buffer:
            return "failed_to_create_command_buffer";
         case vertex_buffer_error::failed_to_find_a_suitable_queue:
            return "failed_to_find_a_suitable_queue";
         default:
            return "UNKNOWN";
      }
   }

   struct vertex_buffer_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "core_vertex_buffer";
      }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<vertex_buffer_error>(err));
      }
   };

   inline static const vertex_buffer_error_category m_vertex_buffer_category{};

   auto to_err_code(vertex_buffer_error err) noexcept -> util::error_t
   {
      return {{static_cast<int>(err), m_vertex_buffer_category}};
   }

   auto vertex_buffer::make(create_info&& info) noexcept -> util::result<vertex_buffer>
   {
      const std::size_t size = sizeof(vertex) * std::size(info.vertices);

      info.logger.debug("staging buffer of size {} on memory {}", size,
                        vk::to_string(vk::MemoryPropertyFlagBits::eHostVisible));

      vulkan::buffer staging_buffer{{.device = info.device,
                                     .buffer_size = size,
                                     .usage = vk::BufferUsageFlagBits::eTransferSrc,
                                     .desired_mem_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                        vk::MemoryPropertyFlagBits::eHostCoherent,
                                     .logger = info.logger}};

      {
         void* p_data = info.device.logical().mapMemory(staging_buffer.memory(), 0, size, {});
         memcpy(p_data, info.vertices.data(), size);
         info.device.logical().unmapMemory(staging_buffer.memory());
      }

      vulkan::buffer vertex_buffer{
         {.device = info.device,
          .buffer_size = size,
          .usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
          .desired_mem_flags = vk::MemoryPropertyFlagBits::eDeviceLocal,
          .logger = info.logger}};

      info.logger.debug("vertex buffer of size {} on memory {}", size,
                        vk::to_string(vk::MemoryPropertyFlagBits::eDeviceLocal));

      const auto copy_n_create = [&](vk::UniqueCommandBuffer buffer) noexcept {
         info.logger.info("copying data from staging buffer ({}) to vertex buffer ({})",
                          vk::to_string(vk::MemoryPropertyFlagBits::eHostVisible),
                          vk::to_string(vk::MemoryPropertyFlagBits::eDeviceLocal));

         buffer->begin(
            vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
         buffer->copyBuffer(staging_buffer.get(), vertex_buffer.get(),
                            {vk::BufferCopy{}.setSize(size)});
         buffer->end();

         const auto queue = info.device.get_queue(cacao::queue_flag_bits::transfer).value;
         queue.submit({vk::SubmitInfo{}.setCommandBufferCount(1).setPCommandBuffers(&buffer.get())},
                      nullptr);
         queue.waitIdle();

         info.logger.info("vertex buffer created");

         class vertex_buffer buf = {};
         buf.m_buffer = std::move(vertex_buffer);

         return buf;
      };

      return info.command_pool.create_primary_buffer()
         .map_error([&](util::error_t&& err) {
            info.logger.error("transfer cmd buffer error: {}-{}", err.value().category().name(),
                              err.value().message());

            return to_err_code(vertex_buffer_error::failed_to_create_command_buffer);
         })
         .map(copy_n_create);
   }

   auto vertex_buffer::operator->() noexcept -> vulkan::buffer* { return &m_buffer; }
   auto vertex_buffer::operator->() const noexcept -> const vulkan::buffer* { return &m_buffer; }

   auto vertex_buffer::operator*() noexcept -> vulkan::buffer& { return value(); }
   auto vertex_buffer::operator*() const noexcept -> const vulkan::buffer& { return buffer(); }

   auto vertex_buffer::value() noexcept -> vulkan::buffer& { return m_buffer; }
   auto vertex_buffer::buffer() const noexcept -> const vulkan::buffer& { return m_buffer; }
} // namespace cacao
