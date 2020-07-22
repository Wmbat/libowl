#include <vkn/shader.hpp>

namespace vkn
{
   namespace detail
   {
      auto to_string(shader::error err) -> std::string
      {
         using error = shader::error;

         switch (err)
         {
            case error::invalid_filepath:
               return "INVALID_FILEPATH";
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
} // namespace vkn
