#include "core/shader_codex.hpp"

#include <fstream>
#include <util/logger.hpp>

#include <functional>

namespace fs = std::filesystem;

namespace core
{
   /*
   auto shader_codex::get_shader(const std::string& name) noexcept -> vkn::shader&
   {
      return vkn::shader{};
      // return m_shaders.at(name);
   }
   [[nodiscard]] auto shader_codex::get_shader(const std::string& name) const noexcept
      -> const vkn::shader&
   {
      return {};
      // return m_shaders.at(name);
   }
   */

   using builder = shader_codex::builder;

   builder::builder(util::logger* plogger) noexcept : m_plogger{plogger} {}

   auto builder::build() -> core::result<shader_codex>
   {
      shader_codex codex{};

      if (!m_info.shader_directory_path.empty())
      {
         if (fs::exists(m_info.shader_directory_path))
         {
            if (fs::is_directory(m_info.shader_directory_path))
            {
               for (const auto& path : fs::directory_iterator(m_info.shader_directory_path))
               {
                  m_info.shader_paths.push_back(path);
               }
            }
            else
            {
               util::log_warn(m_plogger, "[core] provided path \"{0}\" is not a directory",
                              m_info.shader_directory_path.string());
            }
         }
         else
         {
            util::log_warn(m_plogger, "[core] provided path \"{0}\" does not exist",
                           m_info.shader_directory_path.string());
         }
      }
      else
      {
         util::log_info(m_plogger, "[core] no shader directory provided");
      }

      if (!m_info.shader_paths.empty())
      {
         for ([[maybe_unused]] const auto& path : m_info.shader_paths)
         {
            /*
            const auto result = create_shader(path).right_map([&](vkn::shader shader) {
               return codex.add_precompiled_shader(std::move(shader));
            });

            if (!result.is_right())
            {
               // ERROR
            }
            */
         }
      }
      else
      {
         util::log_warn(m_plogger, "[core] no shaders provided");
      }

      return monad::make_right(std::move(codex));
   }

   auto builder::set_cache_directory(const std::filesystem::path& path) -> builder&
   {
      m_info.cache_directory_path = path;
      return *this;
   }
   auto builder::set_shader_directory(const std::filesystem::path& path) -> builder&
   {
      m_info.shader_directory_path = path;
      return *this;
   }
   auto builder::add_shader_filepath(const std::filesystem::path& path) -> builder&
   {
      m_info.shader_paths.push_back(path);
      return *this;
   }
   auto builder::allow_caching(bool is_caching_allowed) noexcept -> builder&
   {
      m_info.is_caching_allowed = is_caching_allowed;
      return *this;
   }

   auto builder::create_shader(const fs::path& path) -> core::result<vkn::shader>
   {
      if (m_info.is_caching_allowed)
      {
         bool was_cached_before = fs::exists(m_info.shader_directory_path);
         if (was_cached_before)
         {
            std::hash<std::string> hasher;
            const std::string hashed_name = std::to_string(hasher(path.string()));

            fs::path cached_shader_path{};
            for (const fs::path& cache_path : fs::directory_iterator(m_info.shader_directory_path))
            {
               if (hashed_name == cache_path.filename())
               {
                  cached_shader_path = cache_path;
                  break;
               }
            }

            if (!cached_shader_path.empty())
            {
               const auto raw_last_write = fs::last_write_time(path);
               const auto cache_last_write = fs::last_write_time(cached_shader_path);

               if (raw_last_write >= cache_last_write) // NOLINT
               {
                  compile_shader(path);
                  // recompile
               }
               else
               {
                  // create shader with cache
               }
            }
         }
         else
         {
            fs::create_directories(m_info.shader_directory_path);

            compile_shader(path);
            // compile and go
         }
      }
      else
      {
         compile_shader(path);
         // compile and go
      }

      return monad::make_left(std::error_code{});
   }

   auto builder::compile_shader(const std::filesystem::path& path)
      -> core::result<util::dynamic_array<std::uint32_t>>
   {
      std::ifstream file{path};
      if (!file.is_open())
      {
      }

      return monad::make_left(std::error_code{});
   }
} // namespace core
