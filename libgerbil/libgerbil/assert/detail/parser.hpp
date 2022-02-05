#ifndef LIBGERBIL_ASSERT_DETAIL_PARSER_HPP_
#define LIBGERBIL_ASSERT_DETAIL_PARSER_HPP_

#include <optional>
#include <string>

namespace gerbil::inline v0
{
   namespace detail
   {
      enum struct literal_type
      {
         decimal,
         hexadecimal,
         octal,
         binary
      };

      /**
       * @brief Parse an literals (floats and ints) and finds it's format
       *
       * for binary literal:
       *   - prefix: 0b or OB
       *
       * for octal literal:
       *   - prefix: 0
       *
       * for hexadecimal literal:
       *   - prefix: 0x or 0X
       *
       * for decimal literal:
       *   - prefix digit from 1 to 9
       *   - suffix: u U, l L, ll LL, z Z.
       *
       * @param[in] The expression to parse
       *
       * @return
       */
      auto parse_literal_type(const std::string& expr) -> std::optional<literal_type>;

      
   } // namespace detail
} // namespace gerbil::inline v0

#endif // LIBGERBIL_ASSERT_DETAIL_PARSER_HPP_
