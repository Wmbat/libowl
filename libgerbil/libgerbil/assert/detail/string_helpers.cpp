#include <libgerbil/assert/detail/string_helpers.hpp>

#define let const auto

namespace gerbil::inline v0
{
   namespace detail
   {
      auto is_space_character(char c) -> bool
      {
         return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 'f' || c == '\v';
      }
   } // namespace detail
} // namespace gerbil::inline v0
