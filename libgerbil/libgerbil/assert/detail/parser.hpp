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

      enum struct token_type
      {
         keyword,
         punctuation,
         number,
         string,
         named_literal,
         identifier,
         whitespace

      };

      struct parse_token
      {
         token_type type;
         std::string str;
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
      auto parse_literal_type(std::string const& expr) -> std::optional<literal_type>;

      /**
       *
       */
      auto re_match(std::string_view expr) -> std::optional<parse_token>;

      /**
       *
       */
      auto re_match_whitespace(std::string_view expr) -> std::optional<parse_token>;

      /**
       * @brief
       *
       * @param[in] 
       *
       * @return 
       */
      auto re_match_identifier(std::string_view expr) -> std::optional<parse_token>;

      /**
       * @brief
       *
       * @param[in] 
       *
       * @return 
       */
      auto re_match_keyword(std::string_view expr) -> std::optional<parse_token>;

   } // namespace detail
} // namespace gerbil::inline v0

#endif // LIBGERBIL_ASSERT_DETAIL_PARSER_HPP_
