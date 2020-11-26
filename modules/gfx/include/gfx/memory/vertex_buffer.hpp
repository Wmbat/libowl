#pragma once

#include <gfx/commons.hpp>
#include <gfx/data_types.hpp>

#include <util/containers/dynamic_array.hpp>
#include <util/logger.hpp>

#include <vkn/buffer.hpp>
#include <vkn/command_pool.hpp>

namespace gfx
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
         util::dynamic_array<vertex> vertices;

         const vkn::device& device;
         const vkn::command_pool& command_pool;

         util::logger_wrapper logger;
      };

      static auto make(create_info&& info) noexcept -> util::result<vertex_buffer>;

      auto operator->() noexcept -> vkn::buffer*;
      auto operator->() const noexcept -> const vkn::buffer*;

      auto operator*() noexcept -> vkn::buffer&;
      auto operator*() const noexcept -> const vkn::buffer&;

      auto value() noexcept -> vkn::buffer&;
      [[nodiscard]] auto value() const noexcept -> const vkn::buffer&;

   private:
      vkn::buffer m_buffer;
   };
} // namespace gfx

namespace std
{
   template <>
   struct is_error_code_enum<gfx::vertex_buffer_error> : true_type
   {
   };
} // namespace std
