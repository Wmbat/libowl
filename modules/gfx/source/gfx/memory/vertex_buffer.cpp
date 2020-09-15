#include <gfx/memory/vertex_buffer.hpp>

namespace gfx
{
   auto to_string(vertex_buffer_error err) -> std::string
   {
      switch (err)
      {
         case gfx::vertex_buffer_error::failed_to_create_staging_buffer:
            return "failed_to_create_staging_buffer";
         case gfx::vertex_buffer_error::failed_to_create_vertex_buffer:
            return "failed_to_create_vertex_buffer";
         case gfx::vertex_buffer_error::failed_to_create_command_buffer:
            return "failed_to_create_command_buffer";
         case gfx::vertex_buffer_error::failed_to_find_a_suitable_queue:
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

   auto make_error(vertex_buffer_error err) noexcept -> error_t
   {
      return {{static_cast<int>(err), m_vertex_buffer_category}};
   }

   auto vertex_buffer::make(make_info&& info) noexcept -> gfx::result<vertex_buffer>
   {
      const vkn::device& device = *info.p_device;
      const vkn::command_pool& command_pool = *info.p_command_pool;

      const std::size_t size = sizeof(vertex) * std::size(info.vertices);
      const auto map_memory = [&](vkn::buffer&& buffer) noexcept {
         util::log_debug(info.p_logger, "[gfx] mapping vertex data into staging buffer");

         void* p_data = device->mapMemory(buffer.memory(), 0, size, {});
         memcpy(p_data, info.vertices.data(), size);
         device->unmapMemory(buffer.memory());

         return std::move(buffer);
      };
      const auto buffer_error = [&](vkn::error&& err) noexcept {
         util::log_error(info.p_logger, "[gfx] staging buffer error: {}-{}",
                         err.type.category().name(), err.type.message());

         return make_error(vertex_buffer_error::failed_to_create_staging_buffer);
      };

      util::log_debug(info.p_logger, "[gfx] staging buffer of size {} on memory {}", size,
                      vk::to_string(vk::MemoryPropertyFlagBits::eHostVisible));

      auto staging_buffer_res =
         vkn::buffer::builder{device, info.p_logger}
            .set_size(size)
            .set_usage(vk::BufferUsageFlagBits::eTransferSrc)
            .set_desired_memory_type(vk::MemoryPropertyFlagBits::eHostVisible |
                                     vk::MemoryPropertyFlagBits::eHostCoherent)
            .build()
            .map(map_memory)
            .map_error(buffer_error);

      if (!staging_buffer_res)
      {
         return monad::make_error(*staging_buffer_res.error());
      }

      util::log_debug(info.p_logger, "[gfx] vertex buffer of size {} on memory {}", size,
                      vk::to_string(vk::MemoryPropertyFlagBits::eDeviceLocal));

      auto vertex_buffer_res = vkn::buffer::builder{device, info.p_logger}
                                  .set_size(size)
                                  .set_usage(vk::BufferUsageFlagBits::eTransferDst |
                                             vk::BufferUsageFlagBits::eVertexBuffer)
                                  .set_desired_memory_type(vk::MemoryPropertyFlagBits::eDeviceLocal)
                                  .build()
                                  .map_error(buffer_error);

      if (!vertex_buffer_res)
      {
         return monad::make_error(*vertex_buffer_res.error());
      }

      auto staging_buffer = *std::move(staging_buffer_res).value();
      auto vertex_buffer = *std::move(vertex_buffer_res).value();

      const auto copy_n_create = [&](vk::UniqueCommandBuffer buffer) noexcept {
         util::log_info(info.p_logger,
                        "[gfx] copying data from staging buffer ({}) to vertex buffer ({})",
                        vk::to_string(vk::MemoryPropertyFlagBits::eHostVisible),
                        vk::to_string(vk::MemoryPropertyFlagBits::eDeviceLocal));

         buffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
         buffer->copyBuffer(vkn::value(staging_buffer), vkn::value(vertex_buffer),
                            {{.size = size}});
         buffer->end();

         return device.get_queue(vkn::queue::type::graphics)
            .map_error([&](vkn::error&& err) {
               util::log_error(info.p_logger, "[core] no queue found for transfer : {}-{}",
                               err.type.category().name(), err.type.message());

               return make_error(vertex_buffer_error::failed_to_find_a_suitable_queue);
            })
            .map([&](vk::Queue queue) noexcept -> class vertex_buffer {
               queue.submit({{.commandBufferCount = 1, .pCommandBuffers = &buffer.get()}}, nullptr);
               queue.waitIdle();

               util::log_info(info.p_logger, "[gfx] vertex buffer created");

               class vertex_buffer buf = {};
               buf.m_buffer = std::move(vertex_buffer);

               return buf;
            });
      };

      return command_pool.create_primary_buffer()
         .map_error([&](vkn::error&& err) {
            util::log_error(info.p_logger, "[gfx] transfer cmd buffer error: {}-{}",
                            err.type.category().name(), err.type.message());

            return make_error(vertex_buffer_error::failed_to_create_command_buffer);
         })
         .and_then(copy_n_create);
   }

   auto vertex_buffer::operator->() noexcept -> vkn::buffer* { return &m_buffer; }
   auto vertex_buffer::operator->() const noexcept -> const vkn::buffer* { return &m_buffer; }

   auto vertex_buffer::operator*() noexcept -> vkn::buffer& { return value(); }
   auto vertex_buffer::operator*() const noexcept -> const vkn::buffer& { return value(); }

   auto vertex_buffer::value() noexcept -> vkn::buffer& { return m_buffer; }
   auto vertex_buffer::value() const noexcept -> const vkn::buffer& { return m_buffer; }
} // namespace gfx
