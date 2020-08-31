#pragma once

#include <core/core.hpp>

#include <util/containers/dense_hash_map.hpp>

#include <vkn/shader.hpp>

#include <glslang/Public/ShaderLang.h>

namespace core
{
   enum class shader_codex_error
   {
      failed_to_open_file,
      unknow_shader_type,
      failed_to_preprocess_shader,
      failed_to_parse_shader,
      failed_to_link_shader,
      failed_to_cache_shader,
      failed_to_create_shader
   };

   auto to_string(shader_codex_error err) -> std::string;
   auto make_error(shader_codex_error err) noexcept -> error_t;

   class shader_codex
   {
   public:
      shader_codex() = default;

      auto add_shader(const std::filesystem::path& path) -> std::string;
      auto add_precompiled_shader(vkn::shader&& shader) -> std::string;

      auto get_shader(const std::string& name) noexcept -> vkn::shader&;
      [[nodiscard]] auto get_shader(const std::string& name) const noexcept -> const vkn::shader&;

   private:
      std::unordered_map<std::string, vkn::shader> m_shaders;

      static inline constexpr int client_input_semantics_version = 100;
      static inline constexpr int default_version = 100;

   public:
      class builder
      {
      public:
         builder(const vkn::device& device, util::logger* plogger) noexcept;

         auto build() -> core::result<shader_codex>;

         auto set_cache_directory(const std::filesystem::path& path) -> builder&;
         auto set_shader_directory(const std::filesystem::path& path) -> builder&;
         auto add_shader_filepath(const std::filesystem::path& path) -> builder&;

         auto allow_caching(bool is_caching_allowed = true) noexcept -> builder&;

      private:
         auto create_shader(const std::filesystem::path& path) -> core::result<vkn::shader>;
         auto compile_shader(const std::filesystem::path& path)
            -> core::result<util::dynamic_array<std::uint32_t>>;
         auto load_shader(const std::filesystem::path& path)
            -> core::result<util::dynamic_array<std::uint32_t>>;

         [[nodiscard]] auto cache_shader(const std::filesystem::path& path,
                                         const util::dynamic_array<std::uint32_t>& data) const
            -> monad::maybe<error_t>;

         [[nodiscard]] auto get_shader_stage(std::string_view stage_name) const -> EShLanguage;
         [[nodiscard]] auto get_spirv_version(uint32_t version) const
            -> glslang::EShTargetLanguageVersion;
         [[nodiscard]] auto get_vulkan_version(uint32_t version) const
            -> glslang::EshTargetClientVersion;
         [[nodiscard]] auto get_shader_type(std::string_view ext_name) const -> vkn::shader::type;

      private:
         util::logger* m_plogger;
         const vkn::device* m_pdevice;

         struct info
         {
            std::filesystem::path cache_directory_path{"cache/shaders"};
            std::filesystem::path shader_directory_path{};
            util::dynamic_array<std::filesystem::path> shader_paths{};

            bool is_caching_allowed = true;
         } m_info;
      };
   };
} // namespace core
