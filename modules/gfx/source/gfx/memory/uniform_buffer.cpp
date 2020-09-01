#include <gfx/memory/uniform_buffer.hpp>

namespace gfx
{
   auto to_string(uniform_buffer_error err) -> std::string
   {
      switch (err)
      {
         case uniform_buffer_error::failed_to_create_staging_buffer:
            return "failed_to_create_staging_buffer";
         case uniform_buffer_error::failed_to_create_uniform_buffer:
            return "failed_to_create_uniform_buffer";
         case uniform_buffer_error::failed_to_create_command_buffer:
            return "failed_to_create_command_buffer";
         case uniform_buffer_error::failed_to_find_a_suitable_queue:
            return "failed_to_find_a_suitable_queue";
         default:
            return "UNKNOWN";
      }
   }

   struct uniform_buffer_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "core_uniform_buffer";
      }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<uniform_buffer_error>(err));
      }
   };
   inline static const uniform_buffer_error_category m_uniform_buffer_category{};

   auto make_error(uniform_buffer_error err) noexcept -> error_t
   {
      return {{static_cast<int>(err), m_uniform_buffer_category}};
   }

   auto uniform_buffer::make(create_info&& info) noexcept -> gfx::result<uniform_buffer>
   {
      const vkn::device& device = *info.p_device;

      const std::size_t size = sizeof(info.indices[0]) * std::size(info.indices);

      const auto map_memory = [&](vkn::buffer&& buffer) noexcept {
         void* p_data = device->mapMemory(buffer.memory(), 0, size, {});
         memcpy(p_data, info.indices.data(), size);
         device->unmapMemory(buffer.memory());

         return std::move(buffer);
      };

      return vkn::buffer::builder{device, info.p_logger}
         .set_size(size)
         .set_usage(vk::BufferUsageFlagBits::eUniformBuffer)
         .set_desired_memory_type(vk::MemoryPropertyFlagBits::eHostVisible |
                                  vk::MemoryPropertyFlagBits::eHostCoherent)
         .build()
         .map(map_memory)
         .map([](vkn::buffer&& buffer) noexcept {
            class uniform_buffer uniform = {};
            uniform.m_buffer = std::move(buffer);

            return uniform;
         })
         .map_error([&](vkn::error&& err) noexcept {
            util::log_error(info.p_logger, "[core] staging buffer error: {}-{}",
                            err.type.category().name(), err.type.message());

            return make_error(uniform_buffer_error::failed_to_create_staging_buffer);
         });
   }

   auto uniform_buffer::operator->() noexcept -> vkn::buffer* { return &m_buffer; }
   auto uniform_buffer::operator->() const noexcept -> const vkn::buffer* { return &m_buffer; }

   auto uniform_buffer::operator*() noexcept -> vkn::buffer& { return value(); }
   auto uniform_buffer::operator*() const noexcept -> const vkn::buffer& { return value(); }

   auto uniform_buffer::value() noexcept -> vkn::buffer& { return m_buffer; }
   auto uniform_buffer::value() const noexcept -> const vkn::buffer& { return m_buffer; }

} // namespace gfx
