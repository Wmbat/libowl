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
   class shader
   {
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

   public:
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

      /**
       * The information necessary for the creation of a shader instance
       */
      struct create_info
      {
         vk::Device device{};
         vk::ShaderModule shader_module{};
         shader::type type{};
      };

   public:
      shader() = default;
      shader(const create_info& info);
      shader(create_info&& info);
      shader(const shader&) = delete;
      shader(shader&& other) noexcept;
      ~shader();

      auto operator=(const shader&) -> shader& = delete;
      auto operator=(shader&& rhs) noexcept -> shader&;

      /**
       * Get a reference to the underlying vulkan shader module
       */
      auto value() noexcept -> vk::ShaderModule&;
      /**
       * Get a const reference to the underlying vulkan shader module
       */
      [[nodiscard]] auto value() const noexcept -> const vk::ShaderModule&;
      /**
       * Get the name of the shader
       */
      [[nodiscard]] auto name() const noexcept -> std::string_view;
      /**
       * Get the type of the shader
       */
      [[nodiscard]] auto stage() const noexcept -> type;

      /**
       * Turns the error enum values into an std::error_code
       */
      inline static auto make_error_code(error err) -> std::error_code
      {
         return {static_cast<int>(err), m_category};
      }

   private:
      vk::Device m_device;
      vk::ShaderModule m_shader_module;

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
   };
} // namespace vkn
