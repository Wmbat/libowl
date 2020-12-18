#pragma once

#include <gfx/commons.hpp>
#include <gfx/data_types.hpp>

#include <util/logger.hpp>

#include <vkn/buffer.hpp>
#include <vkn/command_pool.hpp>

#include <libcaramel/containers/dynamic_array.hpp>

namespace gfx
{
   enum struct camera_buffer_error
   {
      failed_to_create_uniform_buffer
   };

   auto to_string(camera_buffer_error err) -> std::string;
   auto make_error(camera_buffer_error err) noexcept -> error_t;

   class camera_buffer
   {
   public:
      struct create_info
      {
         const vkn::device* p_device{};
         util::logger_wrapper logger{};
      };

      static auto make(create_info&& info) noexcept -> gfx::result<camera_buffer>;

      void update_data() noexcept;

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
   struct is_error_code_enum<gfx::camera_buffer_error> : true_type
   {
   };
} // namespace std
