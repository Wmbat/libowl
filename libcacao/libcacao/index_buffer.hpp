#ifndef LIBCACAO_INDEX_BUFFER_HPP
#define LIBCACAO_INDEX_BUFFER_HPP

#include <libcacao/buffer.hpp>
#include <libcacao/command_pool.hpp>
#include <libcacao/device.hpp>
#include <libcacao/export.hpp>

#include <libmannele/core.hpp>

namespace cacao
{
   struct LIBCACAO_SYMEXPORT index_buffer_create_info
   {
      const cacao::device& device;
      const cacao::command_pool& pool;

      std::span<const mannele::u32> indices;

      util::log_ptr logger;
   };

   class LIBCACAO_SYMEXPORT index_buffer
   {
   public:
      index_buffer(const index_buffer_create_info& info);

      [[nodiscard]] auto buffer() const noexcept -> const cacao::buffer&;
      auto buffer() noexcept -> cacao::buffer&;

      [[nodiscard]] auto index_count() const noexcept -> mannele::u64;

   private:
      mannele::u64 m_index_count;

      cacao::buffer m_buffer;

      util::log_ptr m_logger;
   };
} // namespace cacao

#endif // LIBCACAO_INDEX_BUFFER_HPP
