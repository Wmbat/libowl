#pragma once

#include <cacao/gfx/data_types.hpp>
#include <cacao/vulkan/buffer.hpp>
#include <cacao/vulkan/command_pool.hpp>

namespace cacao
{
   enum struct vertex_buffer_error
   {
      failed_to_create_staging_buffer,
      failed_to_create_vertex_buffer,
      failed_to_create_command_buffer,
      failed_to_find_a_suitable_queue
   };

   auto to_string(vertex_buffer_error err) -> std::string;
   auto to_err_code(vertex_buffer_error err) noexcept -> util::error_t;

   class vertex_buffer
   {
   public:
      struct create_info
      {
         crl::dynamic_array<vertex> vertices;

         const cacao::device& device;
         const vkn::command_pool& command_pool;

         util::logger_wrapper logger;
      };

      static auto make(create_info&& info) noexcept -> util::result<vertex_buffer>;

      auto operator->() noexcept -> vulkan::buffer*;
      auto operator->() const noexcept -> const vulkan::buffer*;

      auto operator*() noexcept -> vulkan::buffer&;
      auto operator*() const noexcept -> const vulkan::buffer&;

      auto value() noexcept -> vulkan::buffer&;
      [[nodiscard]] auto buffer() const noexcept -> const vulkan::buffer&;

   private:
      vulkan::buffer m_buffer;
   };
} // namespace cacao

namespace std
{
   template <>
   struct is_error_code_enum<cacao::vertex_buffer_error> : true_type
   {
   };
} // namespace std
