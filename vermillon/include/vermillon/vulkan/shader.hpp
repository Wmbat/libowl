/**
 * @file shader.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 18th of July, 2020
 * @copyright MIT License
 */

#pragma once

#include <vermillon/util/logger.hpp>
#include <vermillon/vulkan/device.hpp>

#include <spirv_cross.hpp>

#include <filesystem>
#include <string_view>

namespace vkn
{
   /**
    * Contains all possible error values comming from the shader class.
    */
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

   /**
    * The supported types of shaders
    */
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

   /**
    * Holds all data related to the vulkan shader
    */
   class shader final : public owning_handle<vk::ShaderModule>
   {
      struct shader_data;

   public:
      /**
       * Get the name of the shader
       */
      [[nodiscard]] auto name() const noexcept -> std::string_view;
      /**
       * Get the type of the shader
       */
      [[nodiscard]] auto stage() const noexcept -> shader_type;

      [[nodiscard]] auto get_data() const noexcept -> const shader_data&;

   private:
      shader_type m_type{shader_type::count};

      using shader_input_location_t =
         util::strong_type<std::uint32_t, struct input_location, util::arithmetic>;

      using shader_uniform_binding_t =
         util::strong_type<std::uint32_t, struct uniform_binding, util::arithmetic>;

      struct shader_data
      {
         crl::dynamic_array<shader_input_location_t> inputs;
         crl::dynamic_array<shader_uniform_binding_t> uniforms;
      } m_data;

      std::string m_name{};

   public:
      /**
       * A helper class to simplify the construction of shader objects
       */
      class builder
      {
      public:
         builder(const device& device, util::logger_wrapper p_logger);

         /**
          * Attempt to construct a shader object using the provided data. If unable to create
          * the shader module, an error will be returned
          */
         auto build() -> util::result<shader>;

         /**
          * Set the compiled SPIRV shader bytecode for the shader module
          */
         auto set_spirv_binary(const crl::dynamic_array<std::uint32_t>& spirv_binary) -> builder&;
         /**
          * Set the name of the shader
          */
         auto set_name(const std::string& name) -> builder&;
         /**
          * Set the type of the shader
          */
         auto set_type(shader_type shader_type) -> builder&;

      private:
         [[nodiscard]] auto create_shader() const noexcept -> util::result<vk::UniqueShaderModule>;

         [[nodiscard]] auto populate_shader_input(const spirv_cross::Compiler& compiler,
                                                  const spirv_cross::ShaderResources& resources)
            -> crl::dynamic_array<shader_input_location_t>;

         [[nodiscard]] auto populate_uniform_buffer(const spirv_cross::Compiler& compiler,
                                                    const spirv_cross::ShaderResources& resources)
            -> crl::dynamic_array<shader_uniform_binding_t>;

      private:
         util::logger_wrapper m_logger{nullptr};

         struct info
         {
            vk::Device device{};
            uint32_t version{0U};

            crl::dynamic_array<std::uint32_t> spirv_binary{};

            shader_type type{shader_type::count};
            std::string name{};
         } m_info;
      };
   };

   auto to_string(shader_error err) -> std::string;
   auto to_err_code(shader_error err) noexcept -> util::error_t;

   auto to_shader_flag(shader_type type) noexcept -> vk::ShaderStageFlags;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::shader_error> : true_type
   {
   };
} // namespace std
