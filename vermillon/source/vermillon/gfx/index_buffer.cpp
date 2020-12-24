#include <vermillon/gfx/index_buffer.hpp>

namespace cacao
{
   auto to_string(index_buffer_error err) -> std::string
   {
      switch (err)
      {
         case index_buffer_error::failed_to_create_staging_buffer:
            return "failed_to_create_staging_buffer";
         case index_buffer_error::failed_to_create_index_buffer:
            return "failed_to_create_index_buffer";
         case index_buffer_error::failed_to_create_command_buffer:
            return "failed_to_create_command_buffer";
         case index_buffer_error::failed_to_find_a_suitable_queue:
            return "failed_to_find_a_suitable_queue";
         default:
            return "UNKNOWN";
      }
   }

   struct index_buffer_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "core_index_buffer";
      }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<index_buffer_error>(err));
      }
   };
   inline static const index_buffer_error_category m_index_buffer_category{};

   auto to_err_code(index_buffer_error err) noexcept -> util::error_t
   {
      return {{static_cast<int>(err), m_index_buffer_category}};
   }

   auto index_buffer::make(create_info&& info) noexcept -> util::result<index_buffer>
   {
      const std::size_t size = sizeof(info.indices.lookup(0)) * std::size(info.indices);
      const auto map_memory = [&](vkn::buffer&& buffer) noexcept {
         void* p_data = info.device.logical().mapMemory(buffer.memory(), 0, size, {});
         memcpy(p_data, info.indices.data(), size);
         info.device.logical().unmapMemory(buffer.memory());

         return std::move(buffer);
      };
      const auto buffer_error = [&](util::error_t&& err) noexcept {
         info.logger.error("staging buffer error: {}-{}", err.value().category().name(),
                           err.value().message());

         return to_err_code(index_buffer_error::failed_to_create_staging_buffer);
      };

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

      auto index_buffer_res = vkn::buffer::builder{info.device, info.logger}
                                 .set_size(size)
                                 .set_usage(vk::BufferUsageFlagBits::eTransferDst |
                                            vk::BufferUsageFlagBits::eIndexBuffer)
                                 .set_desired_memory_type(vk::MemoryPropertyFlagBits::eDeviceLocal)
                                 .build()
                                 .map_error(buffer_error);

      if (!index_buffer_res)
      {
         return monad::err(*index_buffer_res.error());
      }

      auto staging_buffer = *std::move(staging_buffer_res).value();
      auto index_buffer = *std::move(index_buffer_res).value();

      const auto copy_n_create = [&](vk::UniqueCommandBuffer buffer) noexcept {
         buffer->begin(
            vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
         buffer->copyBuffer(staging_buffer.value(), index_buffer.value(),
                            {vk::BufferCopy{}.setSize(size)});
         buffer->end();

         return info.device.get_queue(vkn::queue_type::graphics)
            .map_error([&](util::error_t&& err) {
               info.logger.error("no queue found for transfer : {}-{}",
                                 err.value().category().name(), err.value().message());

               return to_err_code(index_buffer_error::failed_to_find_a_suitable_queue);
            })
            .map([&](vk::Queue queue) noexcept -> class index_buffer {
               queue.submit(
                  {vk::SubmitInfo{}.setCommandBufferCount(1).setPCommandBuffers(&buffer.get())},
                  nullptr);
               queue.waitIdle();

               info.logger.info("index buffer created");

               class index_buffer buf = {};
               buf.m_buffer = std::move(index_buffer);
               buf.m_index_count = std::size(info.indices);

               return buf;
            });
      };

      return info.command_pool.create_primary_buffer()
         .map_error([&](util::error_t&& err) {
            info.logger.error("transfer cmd buffer error: {}-{}", err.value().category().name(),
                              err.value().message());

            return to_err_code(index_buffer_error::failed_to_create_command_buffer);
         })
         .and_then(copy_n_create);
   }

   auto index_buffer::operator->() noexcept -> vkn::buffer* { return &m_buffer; }
   auto index_buffer::operator->() const noexcept -> const vkn::buffer* { return &m_buffer; }

   auto index_buffer::operator*() noexcept -> vkn::buffer& { return value(); }
   auto index_buffer::operator*() const noexcept -> const vkn::buffer& { return value(); }

   auto index_buffer::value() noexcept -> vkn::buffer& { return m_buffer; }
   auto index_buffer::value() const noexcept -> const vkn::buffer& { return m_buffer; }

   auto index_buffer::index_count() const noexcept -> std::size_t { return m_index_count; }
} // namespace cacao
