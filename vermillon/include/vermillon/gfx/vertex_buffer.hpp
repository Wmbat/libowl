#pragma once

#include <vermillon/gfx/data_types.hpp>
#include <vermillon/util/logger.hpp>
#include <vermillon/vulkan/buffer.hpp>
#include <vermillon/vulkan/command_pool.hpp>

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

         const vkn::device& device;
         const vkn::command_pool& command_pool;

         logger_wrapper logger;
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
} // namespace cacao

namespace std
{
   template <>
   struct is_error_code_enum<cacao::vertex_buffer_error> : true_type
   {
   };
} // namespace std
