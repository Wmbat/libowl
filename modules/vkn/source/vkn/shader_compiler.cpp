#include "vkn/shader_compiler.hpp"

namespace vkn
{
   using builder = shader_compiler::builder;

   builder::builder(const device& device, util::logger* const plogger) : m_plogger{plogger}
   {
      m_info.device = device.value();
   }

   auto builder::build() -> result<shader_compiler>
   {
      if (m_info.allow_caching)
      { 
      }
   }

   auto builder::allow_caching(bool allow_caching) -> builder&
   {
      m_info.allow_caching = allow_caching;
      return *this;
   }

   auto builder::set_cache_folder(const std::filesystem::path& cache_filepath) -> builder&
   {
      m_info.cache_filepath = cache_filepath;
      return *this;
   }
}; // namespace vkn
