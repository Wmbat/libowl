#include <vkn/shader.hpp>

#include <monads/try.hpp>

#include <spirv_glsl.hpp>

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

   shader::builder::builder(const device& device, std::shared_ptr<util::logger> p_logger) :
      mp_logger{std::move(p_logger)}
   {
      m_info.device = device.value();
      m_info.version = device.get_vulkan_version();
   }

   auto shader::builder::build() -> result<shader>
   {
      const auto create_info = vk::ShaderModuleCreateInfo{}
                                  .setCodeSize(m_info.spirv_binary.size() * 4)
                                  .setPCode(m_info.spirv_binary.data());

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createShaderModuleUnique(create_info);
             })
         .map_error([](auto&& error) {
            return make_error(shader_error::failed_to_create_shader_module, error.code());
         })
         .map([&](auto&& handle) {
            util::log_info(mp_logger, "[vkn] shader module created");

            shader s{};
            s.m_value = std::move(handle);
            s.m_name = m_info.name;
            s.m_type = m_info.type;

            return s;
         });
   }

   auto shader::builder::set_spirv_binary(const util::dynamic_array<std::uint32_t>& spirv_binary)
      -> builder&
   {
      m_info.spirv_binary = spirv_binary;
      return *this;
   }
   auto shader::builder::set_name(const std::string& name) -> builder&
   {
      m_info.name = name;
      return *this;
   }
   auto shader::builder::set_type(shader_type type) -> builder&
   {
      m_info.type = type;
      return *this;
   }
} // namespace vkn
