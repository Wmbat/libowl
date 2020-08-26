/**
 * @file shader.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 18th of July, 2020
 * @copyright MIT License
 */

#pragma once

#include <vkn/device.hpp>

#include <util/logger.hpp>

#include <filesystem>
#include <string_view>

namespace vkn
{
   /**
    * Holds all data related to the vulkan shader
    */
   class shader final
   {
   public:
      using value_type = vk::ShaderModule;
      using pointer = value_type*;
      using const_pointer = const value_type*;

      /**
       * Contains all possible error values comming from the shader class.
       */
      enum class error
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
      enum class type
      {
         vertex,
         fragment,
         compute,
         geometry,
         tess_eval,
         tess_control,
         count
      };

   public:
      shader() = default;

      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() noexcept -> pointer;
      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() const noexcept -> const_pointer;

      /**
       * Get the underlying handle
       */
      auto operator*() const noexcept -> value_type;

      operator bool() const noexcept;

      /**
       * Get the underlying handle
       */
      [[nodiscard]] auto value() const noexcept -> vk::ShaderModule;
      /**
       * Get the name of the shader
       */
      [[nodiscard]] auto name() const noexcept -> std::string_view;
      /**
       * Get the type of the shader
       */
      [[nodiscard]] auto stage() const noexcept -> type;

   private:
      vk::UniqueShaderModule m_shader_module;

      type m_type{type::count};
      std::string m_name{};

   public:
      /**
       * A helper class to simplify the construction of shader objects
       */
      class builder
      {
      public:
         builder(const device& device, util::logger* const plogger);

         /**
          * Attempt to construct a shader object using the provided data. If unable to create
          * the shader module, an error will be returned
          */
         auto build() -> result<shader>;

         /**
          * Set the compiled SPIRV shader bytecode for the shader module
          */
         auto set_spirv_binary(const util::dynamic_array<std::uint32_t>& spirv_binary) -> builder&;
         /**
          * Set the name of the shader
          */
         auto set_name(const std::string& name) -> builder&;
         /**
          * Set the type of the shader
          */
         auto set_type(type shader_type) -> builder&;

      private:
         util::logger* const m_plogger{nullptr};

         struct info
         {
            vk::Device device{};
            uint32_t version{0u};

            util::dynamic_array<std::uint32_t> spirv_binary{};

            type m_type{type::count};
            std::string name{};
         } m_info;
      };

   private:
      /**
       * The information necessary for the creation of a shader instance
       */
      struct create_info
      {
         vk::UniqueShaderModule shader_module{};

         std::string name{};

         shader::type type{};
      };

      shader(create_info&& info);

      /**
       * A struct used for error handling and displaying error messages
       */
      struct error_category : std::error_category
      {
         /**
          * The name of the vkn object the error appeared from.
          */
         [[nodiscard]] auto name() const noexcept -> const char* override;
         /**
          * Get the message associated with a specific error code.
          */
         [[nodiscard]] auto message(int err) const -> std::string override;
      };

      inline static const error_category m_category{};

      static auto make_error(error flag, std::error_code ec) -> vkn::error
      {
         return vkn::error{{static_cast<int>(flag), m_category},
                           static_cast<vk::Result>(ec.value())};
      };
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::shader::error> : true_type
   {
   };
} // namespace std
