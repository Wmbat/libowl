#include <gfx/memory/camera_buffer.hpp>

namespace gfx
{
   struct camera_buffer_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "core_camera_buffer";
      }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<camera_buffer_error>(err));
      }
   };
   inline static const camera_buffer_error_category m_camera_buffer_category{};

   auto to_string(camera_buffer_error err) -> std::string
   {
      switch (err)
      {
         case camera_buffer_error::failed_to_create_uniform_buffer:
            return "failed_to_create_uniform_buffer";
         default:
            return "UNKNOWN";
      }
   }

   auto make_error(camera_buffer_error err) noexcept -> error_t
   {
      return {{static_cast<int>(err), m_camera_buffer_category}};
   }

   auto camera_buffer::make(create_info&& info) noexcept -> gfx::result<camera_buffer>
   {
      const vkn::device& device = *info.p_device;

      const auto buffer_error = [&](util::error_t&& err) noexcept {
         info.logger.error("[core] uniform buffer error: {}-{}", err.value().category().name(),
                           err.value().message());

         return make_error(camera_buffer_error::failed_to_create_uniform_buffer);
      };

      return vkn::buffer::builder{device, info.logger}
         .set_size(sizeof(gfx::camera_matrices))
         .set_usage(vk::BufferUsageFlagBits::eUniformBuffer)
         .set_desired_memory_type(vk::MemoryPropertyFlagBits::eHostVisible |
                                  vk::MemoryPropertyFlagBits::eHostCoherent)
         .build()
         .map_error(buffer_error)
         .map([&](vkn::buffer&& handle) {
            info.logger.info("[core] camera buffer created");

            camera_buffer buf;
            buf.m_buffer = std::move(handle);

            return buf;
         });
   }

   auto camera_buffer::operator->() noexcept -> vkn::buffer* { return &m_buffer; }
   auto camera_buffer::operator->() const noexcept -> const vkn::buffer* { return &m_buffer; }

   auto camera_buffer::operator*() noexcept -> vkn::buffer& { return value(); }
   auto camera_buffer::operator*() const noexcept -> const vkn::buffer& { return value(); }

   auto camera_buffer::value() noexcept -> vkn::buffer& { return m_buffer; }
   auto camera_buffer::value() const noexcept -> const vkn::buffer& { return m_buffer; }
} // namespace gfx
