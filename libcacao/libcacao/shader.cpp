#include <libcacao/shader.hpp>

#include <libreglisse/result.hpp>
#include <libreglisse/try.hpp>

#include <magic_enum.hpp>

#if defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdouble-promotion"
#   pragma GCC diagnostic ignored "-Wdeprecated"
#endif

#include <spirv_glsl.hpp>

#if defined(__GCC__)
#   pragma GCC diagnostic pop
#endif

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

using namespace reglisse;

namespace cacao
{
   class shader_error_category : public std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override { return "cacao_shader"; }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return std::string(magic_enum::enum_name(static_cast<shader_error>(err)));
      }
   };

   inline static const shader_error_category shader_category;

   auto make_error_condition(shader_error code) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(code), shader_category});
   }

   auto extract_shader_input_ids(const spirv_cross::Compiler& compiler,
                                 const spirv_cross::ShaderResources& resources)
      -> std::vector<std::uint32_t>
   {
      // clang-format off

      return resources.stage_inputs 
         | ranges::views::transform([&](const spirv_cross::Resource& input) {
                return compiler.get_decoration(input.id, spv::DecorationLocation);
             }) 
         | ranges::to<std::vector>;

      // clang-format on
   }

   auto extract_uniform_buffer_ids(const spirv_cross::Compiler& compiler,
                                   const spirv_cross::ShaderResources& resources)
      -> std::vector<std::uint32_t>
   {
      // clang-format off

      return resources.uniform_buffers
         | ranges::views::transform([&](const spirv_cross::Resource& input) {
                return compiler.get_decoration(input.id, spv::DecorationLocation);
             }) 
         | ranges::to<std::vector>;

      // clang-format on
   }

   shader::shader(const shader_create_info& info) :
      m_name(info.name), m_type(info.type), m_logger(info.logger)
   {
      const auto glsl = spirv_cross::Compiler(info.binary);
      const auto& resources = glsl.get_shader_resources();

      m_inputs = extract_shader_input_ids(glsl, resources);
      m_uniforms = extract_uniform_buffer_ids(glsl, resources);

      const auto create_info = vk::ShaderModuleCreateInfo{.codeSize = std::size(info.binary),
                                                          .pCode = std::data(info.binary)};

      m_module = info.device.logical().createShaderModuleUnique(create_info);
   }
   shader::shader(shader_create_info&& info) :
      m_name(std::move(info.name)), m_type(info.type), m_logger(info.logger)
   {
      const auto glsl = spirv_cross::Compiler(info.binary);
      const auto& resources = glsl.get_shader_resources();

      m_inputs = extract_shader_input_ids(glsl, resources);
      m_uniforms = extract_uniform_buffer_ids(glsl, resources);

      const auto create_info =
         vk::ShaderModuleCreateInfo{.codeSize = std::size(info.binary) * sizeof(mannele::u32),
                                    .pCode = std::data(info.binary)};

      m_module = info.device.logical().createShaderModuleUnique(create_info);
   }

   auto shader::name() const noexcept -> std::string_view { return m_name; }
   auto shader::module() const noexcept -> vk::ShaderModule { return m_module.get(); }
   auto shader::type() const noexcept -> shader_type { return m_type; }
   auto shader::input_ids() const noexcept -> std::span<const std::uint32_t> { return m_inputs; }
   auto shader::uniform_buffer_ids() const noexcept -> std::span<const std::uint32_t>
   {
      return m_uniforms;
   }

   auto to_shader_flag(shader_type type) noexcept -> vk::ShaderStageFlags
   {
      switch (type)
      {
         case shader_type::vertex:
            return vk::ShaderStageFlagBits::eVertex;
         case shader_type::fragment:
            return vk::ShaderStageFlagBits::eFragment;
         case shader_type::compute:
            return vk::ShaderStageFlagBits::eCompute;
         case shader_type::geometry:
            return vk::ShaderStageFlagBits::eGeometry;
         case shader_type::tess_control:
            return vk::ShaderStageFlagBits::eTessellationControl;
         case shader_type::tess_eval:
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
         default:
            return vk::ShaderStageFlagBits::eAll;
      }
   }
} // namespace cacao
