#ifndef LIBCACAO_VERTEX_BUFFER_HPP
#define LIBCACAO_VERTEX_BUFFER_HPP

#include <libcacao/buffer.hpp>
#include <libcacao/command_pool.hpp>
#include <libcacao/export.hpp>
#include <libcacao/gfx/data_types.hpp>

namespace cacao
{
   struct LIBCACAO_SYMEXPORT vertex_buffer_create_info
   {
      const cacao::device& device;
      const cacao::command_pool& pool;

      std::span<vertex> vertices;

      util::log_ptr logger;
   };

   class LIBCACAO_SYMEXPORT vertex_buffer
   {
   public:
      vertex_buffer(const vertex_buffer_create_info& info);
      vertex_buffer(vertex_buffer_create_info&& info);

      [[nodiscard]] auto value() const noexcept -> const buffer&;
      auto value() noexcept -> buffer&;

      [[nodiscard]] auto vertex_count() const noexcept -> mannele::u64;

   private:
      mannele::u64 m_vertex_count;

      buffer m_buffer;

      util::log_ptr m_logger;
   };
} // namespace cacao

#endif // LIBCACAO_VERTEX_BUFFER_HPP
