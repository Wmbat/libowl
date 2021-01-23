#pragma once

#include <cacao/gfx/data_types.hpp>
#include <cacao/vulkan/buffer.hpp>
#include <cacao/vulkan/command_pool.hpp>

#include <utility/logger.hpp>

namespace cacao
{
   enum struct index_buffer_error
   {
      failed_to_create_staging_buffer,
      failed_to_create_index_buffer,
      failed_to_create_command_buffer,
      failed_to_find_a_suitable_queue
   };

   auto to_string(index_buffer_error err) -> std::string;
   auto to_err_code(index_buffer_error err) noexcept -> util::error_t;

   class index_buffer
   {
   public:
      struct create_info
      {
         crl::dynamic_array<uint32_t> indices;

         const cacao::device& device;
         const vkn::command_pool& command_pool;

         util::logger_wrapper logger;
      };

      static auto make(create_info&& info) noexcept -> util::result<index_buffer>;

      auto operator->() noexcept -> vulkan::buffer*;
      auto operator->() const noexcept -> const vulkan::buffer*;

      auto operator*() noexcept -> vulkan::buffer&;
      auto operator*() const noexcept -> const vulkan::buffer&;

      auto value() noexcept -> vulkan::buffer&;
      [[nodiscard]] auto value() const noexcept -> const vulkan::buffer&;

      auto index_count() const noexcept -> std::size_t; // NOLINT

   private:
      vulkan::buffer m_buffer;

      std::size_t m_index_count;
   };
} // namespace cacao

namespace std
{
   template <>
   struct is_error_code_enum<cacao::index_buffer_error> : true_type
   {
   };
} // namespace std
