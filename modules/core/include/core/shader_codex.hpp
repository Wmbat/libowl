#pragma once

#include "core/core.hpp"

#include <util/containers/dense_hash_map.hpp>

#include <vkn/shader.hpp>

#include <glslang/Public/ShaderLang.h>

namespace core
{
   class shader_codex
   {
   public:
      shader_codex() = default;
      shader_codex(const shader_codex&) = delete;
      shader_codex(shader_codex&&) = default;
      ~shader_codex() = default;

      auto operator=(const shader_codex&) -> shader_codex& = delete;
      auto operator=(shader_codex &&) -> shader_codex& = default;

      auto add_shader(vkn::shader shader) -> std::string;
      auto add_precompiled_shader(vkn::shader shader) -> std::string;

      auto get_shader(const std::string& name) noexcept -> vkn::shader&;
      [[nodiscard]] auto get_shader(const std::string& name) const noexcept -> const vkn::shader&;

   private:
      util::dense_hash_map<std::string, vkn::shader> m_shaders;

      static inline constexpr int client_input_semantics_version = 100;
      static inline constexpr int default_version = 100;

   public:
      class builder
      {
      public:
         builder(util::logger* plogger) noexcept;

         auto build() -> core::result<shader_codex>;

         auto set_cache_directory(const std::filesystem::path& path) -> builder&;
         auto set_shader_directory(const std::filesystem::path& path) -> builder&;
         auto add_shader_filepath(const std::filesystem::path& path) -> builder&;

         auto allow_caching(bool is_caching_allowed = true) noexcept -> builder&;

      private:
         auto create_shader(const std::filesystem::path& path) -> core::result<vkn::shader>;
         auto compile_shader(const std::filesystem::path& path)
            -> core::result<util::dynamic_array<std::uint32_t>>;

      private:
         util::logger* m_plogger;

         struct info
         {
            std::filesystem::path cache_directory_path{"cache/shaders/"};
            std::filesystem::path shader_directory_path{};
            util::dynamic_array<std::filesystem::path> shader_paths{};

            bool is_caching_allowed = true;
         } m_info;
      };
   };
} // namespace core
