#include <vkn/shader.hpp>

#include <monads/try.hpp>

#include <spirv_cross.hpp>

#include <fstream>

namespace vkn
{
   /**
    * A struct used for error handling and displaying error messages
    */
   struct shader_error_category : std::error_category
   {
      /**
       * The name of the vkn object the error appeared from.
       */
      [[nodiscard]] auto name() const noexcept -> const char* override { return "vkn_shader"; }
      /**
       * Get the message associated with a specific error code.
       */
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<shader_error>(err));
      }
   };

   inline static const shader_error_category shader_category{};

   auto to_string(shader_error flag) -> std::string
   {
      switch (flag)
      {
         case shader_error::no_filepath:
            return "NO_FILEPATH";
         case shader_error::invalid_filepath:
            return "INVALID_FILEPATH";
         case shader_error::filepath_not_a_file:
            return "FILEPATH_NOT_A_FILE";
         case shader_error::failed_to_open_file:
            return "FAILED_TO_OPEN_FILE";
         case shader_error::failed_to_preprocess_shader:
            return "FAILED_TO_PREPROCESS_SHADER";
         case shader_error::failed_to_parse_shader:
            return "FAILED_TO_PARSE_SHADER";
         case shader_error::failed_to_link_shader:
            return "FAILED_TO_LINK_SHADER";
         case shader_error::failed_to_create_shader_module:
            return "FAILED_TO_CREATE_SHADER_MODULE";
         default:
            return "UNKNOWN";
      }
   };

   auto make_error(shader_error flag, std::error_code ec) noexcept -> vkn::error
   {
      return vkn::error{{static_cast<int>(flag), shader_category},
                        static_cast<vk::Result>(ec.value())};
   };

   auto shader::name() const noexcept -> std::string_view { return m_name; }
   auto shader::stage() const noexcept -> shader_type { return m_type; }

   auto shader::get_data() const noexcept -> const shader_data& { return m_data; }

   using builder = shader::builder;

   builder::builder(const device& device, std::shared_ptr<util::logger> p_logger) :
      mp_logger{std::move(p_logger)}
   {
      m_info.device = device.value();
      m_info.version = device.get_vulkan_version();
   }

   auto builder::build() -> result<shader>
   {
      spirv_cross::Compiler glsl{{std::begin(m_info.spirv_binary), std::end(m_info.spirv_binary)}};
      const auto resources = glsl.get_shader_resources();

      const shader_data shader_data{.inputs = populate_shader_input(glsl, resources),
                                    .uniforms = populate_uniform_buffer(glsl, resources)};

      return create_shader().map([&](auto&& handle) {
         util::log_info(mp_logger, "[vkn] shader module created");

         shader s{};
         s.m_value = std::move(handle);
         s.m_name = m_info.name;
         s.m_data = shader_data;
         s.m_type = m_info.type;

         return s;
      });
   }

   auto builder::set_spirv_binary(const util::dynamic_array<std::uint32_t>& spirv_binary)
      -> builder&
   {
      m_info.spirv_binary = spirv_binary;
      return *this;
   }
   auto builder::set_name(const std::string& name) -> builder&
   {
      m_info.name = name;
      return *this;
   }
   auto builder::set_type(shader_type type) -> builder&
   {
      m_info.type = type;
      return *this;
   }

   auto builder::create_shader() const noexcept -> vkn::result<vk::UniqueShaderModule>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createShaderModuleUnique(
                   {.codeSize = std::size(m_info.spirv_binary) * 4,
                    .pCode = std::data(m_info.spirv_binary)});
             })
         .map_error([](auto&& error) {
            return make_error(shader_error::failed_to_create_shader_module, error.code());
         });
   }

   auto shader::builder::populate_shader_input(const spirv_cross::Compiler& compiler,
                                               const spirv_cross::ShaderResources& resources) const
      -> util::dynamic_array<shader_input_location_t>
   {
      util::dynamic_array<shader_input_location_t> res;
      res.reserve(resources.stage_inputs.size());

      for (const auto& input : resources.stage_inputs)
      {
         std::uint32_t location = compiler.get_decoration(input.id, spv::DecorationLocation);

         util::log_debug(mp_logger, R"([vkn] input "{}" with location {})", input.name, location);

         res.emplace_back(shader_input_location_t{location});
      }

      return res;
   }

   auto builder::populate_uniform_buffer([[maybe_unused]] const spirv_cross::Compiler& compiler,
                                         const spirv_cross::ShaderResources& resources) const
      -> util::dynamic_array<shader_uniform_binding_t>
   {
      util::dynamic_array<shader_uniform_binding_t> data{};

      for (const auto& uniform : resources.uniform_buffers)
      {
         std::uint32_t binding = compiler.get_decoration(uniform.id, spv::DecorationBinding);

         util::log_debug(mp_logger, R"([vkn] uniform buffer "{}" with location {})", uniform.name,
                         binding);

         data.emplace_back(shader_uniform_binding_t{binding});
      }

      return data;
   }
} // namespace vkn
