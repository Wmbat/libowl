#include <vkn/shader.hpp>

#include <monads/try.hpp>

#include <fstream>

namespace vkn
{
   namespace detail
   {
      auto to_string(shader::error err) -> std::string
      {
         using error = shader::error;

         switch (err)
         {
            case error::no_filepath:
               return "NO_FILEPATH";
            case error::invalid_filepath:
               return "INVALID_FILEPATH";
            case error::filepath_not_a_file:
               return "FILEPATH_NOT_A_FILE";
            case error::failed_to_open_file:
               return "FAILED_TO_OPEN_FILE";
            case error::failed_to_preprocess_shader:
               return "FAILED_TO_PREPROCESS_SHADER";
            case error::failed_to_parse_shader:
               return "FAILED_TO_PARSE_SHADER";
            case error::failed_to_link_shader:
               return "FAILED_TO_LINK_SHADER";
            case error::failed_to_create_shader_module:
               return "FAILED_TO_CREATE_SHADER_MODULE";
            default:
               return "UNKNOWN";
         }
      };
   } // namespace detail

   auto shader::error_category::name() const noexcept -> const char* { return "vk_shader"; }
   auto shader::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<shader::error>(err));
   }

   shader::shader(create_info&& info) :
      m_shader_module{std::move(info.shader_module)}, m_type{info.type}, m_name{
                                                                            std::move(info.name)}
   {}

   auto shader::operator->() noexcept -> pointer { return &m_shader_module.get(); }
   auto shader::operator->() const noexcept -> const_pointer { return &m_shader_module.get(); }

   auto shader::operator*() const noexcept -> value_type { return value(); }

   shader::operator bool() const noexcept { return m_shader_module.get(); }

   auto shader::value() const noexcept -> vk::ShaderModule { return m_shader_module.get(); }
   auto shader::name() const noexcept -> std::string_view { return m_name; }
   auto shader::stage() const noexcept -> type { return m_type; }

   shader::builder::builder(const device& device, util::logger* const plogger) : m_plogger{plogger}
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
            return make_error(error::failed_to_create_shader_module, error.code());
         })
         .map([&](auto&& handle) {
            util::log_info(m_plogger, "[vkn] shader module created");

            return shader{shader::create_info{
               .shader_module = std::move(handle), .name = m_info.name, .type = m_info.m_type}};
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
   auto shader::builder::set_type(type shader_type) -> builder&
   {
      m_info.m_type = shader_type;
      return *this;
   }
} // namespace vkn
