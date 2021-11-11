/**
 * @file libcacao/shader.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_SHADER_HPP_
#define LIBCACAO_SHADER_HPP_

#include <libcacao/device.hpp>
#include <libcacao/export.hpp>

// Engine Libraries

#include <libmannele/logging/logger.hpp>

// C++ Standard Libraries

#include <string>
#include <string_view>
#include <system_error> // NOLINT
#include <vector>

namespace cacao
{
   enum class shader_error
   {
      no_filepath,
      invalid_filepath,
      filepath_not_a_file,
      failed_to_open_file,
      failed_to_preprocess_shader,
      failed_to_parse_shader,
      failed_to_link_shader,
      failed_to_create_shader_module
   };

   auto LIBCACAO_SYMEXPORT make_error_condition(shader_error error) -> std::error_condition;

   enum class shader_type
   {
      vertex,
      fragment,
      compute,
      geometry,
      tess_eval,
      tess_control,
      count
   };

   struct LIBCACAO_SYMEXPORT shader_create_info
   {
      const cacao::device& device;

      std::string name;
      shader_type type;
      std::vector<std::uint32_t> binary;

      mannele::log_ptr logger;
   };

   class LIBCACAO_SYMEXPORT shader
   {
   public:
      explicit shader(const shader_create_info& info);
      explicit shader(shader_create_info&& info);

      [[nodiscard]] auto name() const noexcept -> std::string_view;
      [[nodiscard]] auto module() const noexcept -> vk::ShaderModule;
      [[nodiscard]] auto type() const noexcept -> shader_type;
      [[nodiscard]] auto input_ids() const noexcept -> std::span<const std::uint32_t>;
      [[nodiscard]] auto uniform_buffer_ids() const noexcept -> std::span<const std::uint32_t>;

   private:
      std::string m_name;
      shader_type m_type;

      std::vector<std::uint32_t> m_inputs;
      std::vector<std::uint32_t> m_uniforms;

      vk::UniqueShaderModule m_module;

      mannele::log_ptr m_logger;
   };

   auto to_shader_flag(shader_type type) noexcept -> vk::ShaderStageFlags;
} // namespace cacao

#endif // LIBCACAO_SHADER_HPP_
