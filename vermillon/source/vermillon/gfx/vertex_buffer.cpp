#include <vermillon/gfx/vertex_buffer.hpp>

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
      const auto map_memory = [&](vkn::buffer&& buffer) noexcept {
         info.logger.debug("[gfx] mapping vertex data into staging buffer");

         void* p_data = info.device.logical().mapMemory(buffer.memory(), 0, size, {});
         memcpy(p_data, info.vertices.data(), size);
         info.device.logical().unmapMemory(buffer.memory());

         return std::move(buffer);
      };
      const auto buffer_error = [&](util::error_t&& err) noexcept {
         info.logger.error("[gfx] staging buffer error: {}-{}", err.value().category().name(),
                           err.value().message());

         return to_err_code(vertex_buffer_error::failed_to_create_staging_buffer);
      };

      info.logger.debug("[gfx] staging buffer of size {} on memory {}", size,
                        vk::to_string(vk::MemoryPropertyFlagBits::eHostVisible));

      auto staging_buffer_res =
         vkn::buffer::builder{info.device, info.logger}
            .set_size(size)
            .set_usage(vk::BufferUsageFlagBits::eTransferSrc)
            .set_desired_memory_type(vk::MemoryPropertyFlagBits::eHostVisible |
                                     vk::MemoryPropertyFlagBits::eHostCoherent)
            .build()
            .map(map_memory)
            .map_error(buffer_error);

      if (!staging_buffer_res)
      {
         return monad::err(*staging_buffer_res.error());
      }

      info.logger.debug("[gfx] vertex buffer of size {} on memory {}", size,
                        vk::to_string(vk::MemoryPropertyFlagBits::eDeviceLocal));

      auto vertex_buffer_res = vkn::buffer::builder{info.device, info.logger}
                                  .set_size(size)
                                  .set_usage(vk::BufferUsageFlagBits::eTransferDst |
                                             vk::BufferUsageFlagBits::eVertexBuffer)
                                  .set_desired_memory_type(vk::MemoryPropertyFlagBits::eDeviceLocal)
                                  .build()
                                  .map_error(buffer_error);

      if (!vertex_buffer_res)
      {
         return monad::err(*vertex_buffer_res.error());
      }

      auto staging_buffer = *std::move(staging_buffer_res).value();
      auto vertex_buffer = *std::move(vertex_buffer_res).value();

      const auto copy_n_create = [&](vk::UniqueCommandBuffer buffer) noexcept {
         info.logger.info("[gfx] copying data from staging buffer ({}) to vertex buffer ({})",
                          vk::to_string(vk::MemoryPropertyFlagBits::eHostVisible),
                          vk::to_string(vk::MemoryPropertyFlagBits::eDeviceLocal));

         buffer->begin(
            vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
         buffer->copyBuffer(vkn::value(staging_buffer), vkn::value(vertex_buffer),
                            {vk::BufferCopy{}.setSize(size)});
         buffer->end();

         return info.device.get_queue(vkn::queue_type::graphics)
            .map_error([&](util::error_t&& err) {
               info.logger.error("[core] no queue found for transfer : {}-{}",
                                 err.value().category().name(), err.value().message());

               return to_err_code(vertex_buffer_error::failed_to_find_a_suitable_queue);
            })
            .map([&](vk::Queue queue) noexcept -> class vertex_buffer {
               queue.submit(
                  {vk::SubmitInfo{}.setCommandBufferCount(1).setPCommandBuffers(&buffer.get())},
                  nullptr);
               queue.waitIdle();

               info.logger.info("[gfx] vertex buffer created");

               class vertex_buffer buf = {};
               buf.m_buffer = std::move(vertex_buffer);

               return buf;
            });
      };

      return info.command_pool.create_primary_buffer()
         .map_error([&](util::error_t&& err) {
            info.logger.error("[gfx] transfer cmd buffer error: {}-{}",
                              err.value().category().name(), err.value().message());

            return to_err_code(vertex_buffer_error::failed_to_create_command_buffer);
         })
         .and_then(copy_n_create);
   }

   auto vertex_buffer::operator->() noexcept -> vkn::buffer* { return &m_buffer; }
   auto vertex_buffer::operator->() const noexcept -> const vkn::buffer* { return &m_buffer; }

   auto vertex_buffer::operator*() noexcept -> vkn::buffer& { return value(); }
   auto vertex_buffer::operator*() const noexcept -> const vkn::buffer& { return value(); }

   auto vertex_buffer::value() noexcept -> vkn::buffer& { return m_buffer; }
   auto vertex_buffer::value() const noexcept -> const vkn::buffer& { return m_buffer; }
} // namespace cacao
