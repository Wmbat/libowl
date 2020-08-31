#include <core/graphics/index_buffer.hpp>

namespace core
{
   auto to_string(index_buffer_error err) -> std::string
   {
      switch (err)
      {
         case core::index_buffer_error::failed_to_create_staging_buffer:
            return "failed_to_create_staging_buffer";
         case core::index_buffer_error::failed_to_create_index_buffer:
            return "failed_to_create_index_buffer";
         case core::index_buffer_error::failed_to_create_command_buffer:
            return "failed_to_create_command_buffer";
         case core::index_buffer_error::failed_to_find_a_suitable_queue:
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

   auto make_error(index_buffer_error err) noexcept -> error_t
   {
      return {{static_cast<int>(err), m_index_buffer_category}};
   }

   auto index_buffer::make(create_info&& info) noexcept -> core::result<index_buffer>
   {
      const vkn::device& device = *info.p_device;
      const vkn::command_pool& command_pool = *info.p_command_pool;

      const std::size_t size = sizeof(info.indices[0]) * std::size(info.indices);
      const auto map_memory = [&](vkn::buffer&& buffer) noexcept {
         void* p_data = device->mapMemory(buffer.memory(), 0, size, {});
         memcpy(p_data, info.indices.data(), size);
         device->unmapMemory(buffer.memory());

         return std::move(buffer);
      };
      const auto buffer_error = [&](vkn::error&& err) noexcept {
         util::log_error(info.p_logger, "[core] staging buffer error: {}-{}",
                         err.type.category().name(), err.type.message());

         return make_error(index_buffer_error::failed_to_create_staging_buffer);
      };

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

      auto index_buffer_res = vkn::buffer::builder{device, info.p_logger}
                                 .set_size(size)
                                 .set_usage(vk::BufferUsageFlagBits::eTransferDst |
                                            vk::BufferUsageFlagBits::eIndexBuffer)
                                 .set_desired_memory_type(vk::MemoryPropertyFlagBits::eDeviceLocal)
                                 .build()
                                 .map_error(buffer_error);

      if (!index_buffer_res)
      {
         return monad::make_error(*index_buffer_res.error());
      }

      auto staging_buffer = *std::move(staging_buffer_res).value();
      auto index_buffer = *std::move(index_buffer_res).value();

      const auto copy_n_create = [&](vk::UniqueCommandBuffer buffer) noexcept {
         buffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
         buffer->copyBuffer(vkn::value(staging_buffer), vkn::value(index_buffer), {{.size = size}});
         buffer->end();

         return device.get_queue(vkn::queue::type::graphics)
            .map_error([&](vkn::error&& err) {
               util::log_error(info.p_logger, "[core] no queue found for transfer : {}-{}",
                               err.type.category().name(), err.type.message());

               return make_error(index_buffer_error::failed_to_find_a_suitable_queue);
            })
            .map([&](vk::Queue queue) noexcept -> class index_buffer {
               queue.submit({{.commandBufferCount = 1, .pCommandBuffers = &buffer.get()}}, nullptr);
               queue.waitIdle();

               util::log_info(info.p_logger, "[core] vertex buffer created");

               class index_buffer buf = {};
               buf.m_buffer = std::move(index_buffer);

               return buf;
            });
      };

      return command_pool.create_primary_buffer()
         .map_error([&](vkn::error&& err) {
            util::log_error(info.p_logger, "[core] transfer cmd buffer error: {}-{}",
                            err.type.category().name(), err.type.message());

            return make_error(index_buffer_error::failed_to_create_command_buffer);
         })
         .and_then(copy_n_create);
   }

   auto index_buffer::operator->() noexcept -> vkn::buffer* { return &m_buffer; }
   auto index_buffer::operator->() const noexcept -> const vkn::buffer* { return &m_buffer; }

   auto index_buffer::operator*() noexcept -> vkn::buffer& { return value(); }
   auto index_buffer::operator*() const noexcept -> const vkn::buffer& { return value(); }

   auto index_buffer::value() noexcept -> vkn::buffer& { return m_buffer; }
   auto index_buffer::value() const noexcept -> const vkn::buffer& { return m_buffer; }

} // namespace core
