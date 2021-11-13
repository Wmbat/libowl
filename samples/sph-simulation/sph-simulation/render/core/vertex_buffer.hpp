#ifndef SPH_SIMULATION_RENDER_CORE_VERTEX_BUFFER_HPP
#define SPH_SIMULATION_RENDER_CORE_VERTEX_BUFFER_HPP

#include <sph-simulation/data_types/vertex.hpp>

#include <libcacao/buffer.hpp>
#include <libcacao/command_pool.hpp>
#include <libcacao/export.hpp>

struct LIBCACAO_SYMEXPORT vertex_buffer_create_info
{
   const cacao::device& device;
   const cacao::command_pool& pool;

   std::span<const vertex> vertices;

   mannele::log_ptr logger;
};

class LIBCACAO_SYMEXPORT vertex_buffer
{
public:
   vertex_buffer(const vertex_buffer_create_info& info);

   [[nodiscard]] auto buffer() const noexcept -> const cacao::buffer&;
   auto buffer() noexcept -> cacao::buffer&;

   [[nodiscard]] auto vertex_count() const noexcept -> mannele::u64;

private:
   mannele::u64 m_vertex_count;

   cacao::buffer m_buffer;

   mannele::log_ptr m_logger;
};

#endif // SPH_SIMULATION_RENDER_CORE_VERTEX_BUFFER_HPP
