#include "vkn/shader.hpp"

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

   shader::shader(const create_info& info) :
      m_device{info.device}, m_shader_module{info.shader_module}, m_type{info.type}, m_name{
                                                                                        info.name}
   {}
   shader::shader(create_info&& info) :
      m_device{info.device},
      m_shader_module{info.shader_module}, m_type{info.type}, m_name{std::move(info.name)}
   {}
   shader::shader(shader&& other) noexcept { *this = std::move(other); }
   shader::~shader()
   {
      if (m_device && m_shader_module)
      {
         m_device.destroyShaderModule(m_shader_module);

         m_shader_module = nullptr;
         m_device = nullptr;
      }
   }

   auto vkn::shader::operator=(shader&& rhs) noexcept -> shader&
   {
      if (this != &rhs)
      {
         std::swap(m_device, rhs.m_device);
         std::swap(m_shader_module, rhs.m_shader_module);
         std::swap(m_name, rhs.m_name);

         m_type = rhs.m_type;
         rhs.m_type = shader::type::count;
      }

      return *this;
   }

   auto shader::value() const noexcept -> value_type { return m_shader_module; }
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
                                  .setPNext(nullptr)
                                  .setFlags({})
                                  .setCodeSize(m_info.spirv_binary.size() * 4)
                                  .setPCode(m_info.spirv_binary.data());

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createShaderModule(create_info);
             })
         .left_map([](auto&& error) {
            return vkn::error{shader::make_error_code(error::failed_to_create_shader_module),
                              static_cast<vk::Result>(error.code().value())};
         })
         .right_map([&](auto&& handle) {
            util::log_info(m_plogger, "[vkn] shader module created");

            return shader{shader::create_info{.device = m_info.device,
                                              .shader_module = handle,
                                              .name = m_info.name,
                                              .type = m_info.m_type}};
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
