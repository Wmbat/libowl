#pragma once

#include <util/containers/dense_hash_map.hpp>

#include <vkn/shader.hpp>

namespace core
{
   class shader_codex
   {
   public:
   private:
      util::dense_hash_map<std::string, vkn::shader> m_shaders;

   public:
      class builder
      {
      public:
         builder(util::logger* plogger);

         auto set_directory(const std::filesystem::path& path) -> builder&;
         auto add_shader_filepath(const std::filesystem::path& path) -> builder&;

         auto allow_caching(bool is_caching_allowed = true) noexcept -> builder&;

      private:
         util::logger* m_plogger;

         struct info
         {
            std::filesystem::path directory_path;
            util::dynamic_array<std::filesystem::path> shader_paths;

            bool is_caching_allowed = true;
         } m_info;
      };
   };
} // namespace core
