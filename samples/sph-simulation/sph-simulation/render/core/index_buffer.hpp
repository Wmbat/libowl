#ifndef SPH_SIMULATION_RENDER_CORE_INDEX_BUFFER_HPP_
#define SPH_SIMULATION_RENDER_CORE_INDEX_BUFFER_HPP_

#include <libcacao/buffer.hpp>
#include <libcacao/command_pool.hpp>
#include <libcacao/device.hpp>
#include <libcacao/export.hpp>

#include <libmannele/core.hpp>

struct LIBCACAO_SYMEXPORT index_buffer_create_info
{
   const cacao::device& device;
   const cacao::command_pool& pool;

   std::span<const mannele::u32> indices;

   mannele::log_ptr logger;
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

   mannele::log_ptr m_logger;
};

#endif // SPH_SIMULATION_RENDER_CORE_INDEX_BUFFER_HPP_
