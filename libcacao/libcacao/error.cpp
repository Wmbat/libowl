#include <libcacao/error.hpp>

namespace cacao
{
   inline static const error_category err_category{};

   auto error_category::name() const noexcept -> const char* { return "cacao"; }
   auto error_category::message(int err) const -> std::string
   {
      return std::string(magic_enum::enum_name(static_cast<error_code>(err)));
   }

   auto to_error_condition(error_code code) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(code), err_category});
   }
} // namespace cacao
