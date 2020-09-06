#pragma once

#include <gfx/commons.hpp>
#include <gfx/data_types.hpp>

#include <util/containers/dynamic_array.hpp>
#include <util/logger.hpp>

#include <vkn/buffer.hpp>
#include <vkn/command_pool.hpp>

namespace gfx
{
   enum struct uniform_buffer_error
   {
      failed_to_create_staging_buffer,
      failed_to_create_uniform_buffer,
      failed_to_create_command_buffer,
      failed_to_find_a_suitable_queue
   };

   auto to_string(uniform_buffer_error err) -> std::string;
   auto make_error(uniform_buffer_error err) noexcept -> error_t;

   class uniform_buffer
   {
   public:
      struct create_info
      {
         util::dynamic_array<uint32_t> indices;

         vkn::device* p_device;
         std::shared_ptr<util::logger> p_logger;
      };

      static auto make(create_info&& info) noexcept -> gfx::result<uniform_buffer>;

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
   struct is_error_code_enum<gfx::uniform_buffer_error> : true_type
   {
   };
} // namespace std
