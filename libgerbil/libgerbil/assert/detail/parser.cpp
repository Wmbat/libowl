#include <libgerbil/assert/detail/parser.hpp>

#include <ctre.hpp>

#include <algorithm>
#include <array>
#include <ranges>

#if not defined(let)
#   define let auto const
#endif

namespace gerbil::inline v0
{
   namespace
   {
      static constexpr ctll::fixed_string keywords_re_1 =
         R"((alignas|constinit|public|alignof|const_cast)"
         R"(|float|register|try|asm|continue|for|reinterpret_cast)"
         R"(|typedef|auto|co_await|friend|requires|typeid)"
         R"(|bool|co_return|goto|return|typename|break|co_yield)\b)";
      static constexpr ctll::fixed_string keywords_re_2 =
         R"((if|short|union|case|decltype|inline|signed|unsigned)"
         R"(|catch|default|int|sizeof|using|char|delete|long|static)"
         R"(|virtual|char8_t|do|mutable|static_assert|void|char16_t)"
         R"(|double|namespace|static_cast|volatile|char32_t|dynamic_cast)\b)";
      static constexpr ctll::fixed_string keywords_re_3 =
         R"((new|struct|wchar_t|class|else|noexcept|switch|while|concept)"
         R"(|enum|template|const|explicit|operator|this|consteval|export)"
         R"(|private|thread_local|constexpr|extern|protected|throw)\b)";

      // numbers

      // (?:[Ee][\\+-]?\d(?:'?\d)*)

      /*
      static constexpr ctll::fixed_string float_decimal_re = 
         R"((?:{})"
         R"((?:[Ee][\\+-]?\d(?:'?\d)*)?|\d(?:'?\d)*(?:[Ee][\\+-]?\d(?:'?\d)*))[FfLl]?)";
         */
      static constexpr ctll::fixed_string float_hex_re =
         R"(0[Xx](?:(?:(?:[\da-fA-F](?:'?[\da-fA-F])*)?\.[\da-fA-F](?:'?[\da-fA-F])*)"
         R"(|[\da-fA-F](?:'?[\da-fA-F])*\.)|[\da-fA-F](?:'?[\da-fA-F])*))"
         R"([Pp][\+\-]?\d(?:'?\d)*[FfLl]?)";
      static constexpr ctll::fixed_string int_binary_re =
         R"(0[Bb][01](?:'?[01])*(?:[Uu](?:LL?|ll?|Z|z)?|(?:LL?|ll?|Z|z)[Uu]?)?)";
      static constexpr ctll::fixed_string int_octal_re =
         R"(0(?:'?[0-7])+(?:[Uu](?:LL?|ll?|Z|z)?|(?:LL?|ll?|Z|z)[Uu]?)?)";
      static constexpr ctll::fixed_string int_hex_re =
         R"(0[Xx](?!')(?:'?[\da-fA-F])+(?:[Uu](?:LL?|ll?|Z|z)?|(?:LL?|ll?|Z|z)[Uu]?)?)";
      static constexpr ctll::fixed_string int_decimal_re =
         R"((?:0|[1-9](?:'?\d)*)(?:[Uu](?:LL?|ll?|Z|z)?|(?:LL?|ll?|Z|z)[Uu]?)?)";

      // Punctuation

      static constexpr ctll::fixed_string named_literal_re = R"(true|false|nullptr)";

      static constexpr ctll::fixed_string char_literal_re =
         R"((?:u8|[UuL])?'(?:\\[0-7]{1,3}|\\x[\da-fA-F]+|\\.|[^\n'])*')";
      static constexpr ctll::fixed_string string_literal_re =
         R"((?:u8|[UuL])?"(?:\\[0-7]{1,3}|\\x[\da-fA-F]+|\\.|[^\n"])*")";
      static constexpr ctll::fixed_string raw_string_literal_re =
         R"((?:u8|[UuL])?R"([^ ()\\t\r\v\n]*)\((?:(?!\)\").)*\)\")";

      static constexpr ctll::fixed_string identifier_re =
         R"((?!\d+)(?:[\da-zA-Z_\$]|\\u[\da-fA-F]{4}|\\U[\da-fA-F]{8})+)";

      static constexpr ctll::fixed_string whitespace_re = R"(\s+)";

      static constexpr char digit_zero = 0x0030;
      static constexpr char digit_one = 0x0031;
      static constexpr char lowest_digit = digit_zero;
      static constexpr char highest_digit = 0x0039;

      static constexpr char latin_capital_b = 0x0042;
      static constexpr char latin_small_b = 0x0062;

      static constexpr char latin_capital_x = 0x0058;
      static constexpr char latin_small_x = 0x0078;

      /**
       *
       */
      auto is_non_zero_digit(char c) noexcept -> bool
      {
         return c >= digit_one && c <= highest_digit;
      }
      /**
       *
       */
      auto is_digit(char c) noexcept -> bool { return c >= lowest_digit && c <= highest_digit; }

      /**
       *
       */
      auto is_binary_token(char c) noexcept -> bool
      {
         return c == latin_capital_b || c == latin_small_b;
      }
      /**
       *
       */
      auto is_binary_prefix(char first, char second) -> bool
      {
         return first == digit_zero && is_binary_token(second);
      }

      /**
       *
       */
      auto is_hexadecimal_token(char c) noexcept -> bool
      {
         return c == latin_capital_x || c == latin_small_x;
      }
      /**
       *
       */
      auto is_hexadecimal_prefix(char first, char second) -> bool
      {
         return first == digit_zero && is_hexadecimal_token(second);
      }
   } // namespace

   namespace detail
   {

      auto parse_literal_type(const std::string& expr) -> std::optional<literal_type>
      {
         /**
          * We don't need to parse the whole expression, only the beginning
          */

         if (expr.empty())
         {
            return std::nullopt;
         }
         else if (expr.size() == 1 && expr.front() == digit_zero)
         {
            return literal_type::decimal;
         }
         else
         {
            let first = std::begin(expr);
            let second = first + 1;

            if (is_binary_prefix(*first, *second))
            {
               return literal_type::binary;
            }
            else if (is_hexadecimal_prefix(*first, *second))
            {
               return literal_type::hexadecimal;
            }
            else if (*first == digit_zero && is_digit(*second))
            {
               return literal_type::octal;
            }
            else if (is_non_zero_digit(*first))
            {
               return literal_type::decimal;
            }
            else
            {
               return std::nullopt;
            }
         }
      } // namespace detail

      auto re_match_string(std::string_view expr) -> std::optional<parse_token>
      {
         if (auto m = ctre::match<char_literal_re>(expr))
         {
            return parse_token{.type = token_type::string, .str = std::string(m.get<0>())};
         }
         else if (auto m = ctre::match<string_literal_re>(expr))
         {
            return parse_token{.type = token_type::string, .str = std::string(m.get<0>())};
         }
         else if (auto m = ctre::match<raw_string_literal_re>(expr))
         {
            return parse_token{.type = token_type::string, .str = std::string(m.get<0>())};
         }
         else
         {
            return std::nullopt;
         }
      }

      auto re_match_whitespace(std::string_view expr) -> std::optional<parse_token>
      {
         if (auto m = ctre::match<whitespace_re>(expr))
         {
            return parse_token{.type = token_type::identifier, .str = std::string(m.get<0>())};
         }
         else
         {
            return std::nullopt;
         }
      }

      auto re_match_identifier(std::string_view expr) -> std::optional<parse_token>
      {
         if (auto m = ctre::match<identifier_re>(expr))
         {
            return parse_token{.type = token_type::identifier, .str = std::string(m.get<0>())};
         }
         else
         {
            return std::nullopt;
         }
      }

      auto re_match_keyword(std::string_view expr) -> std::optional<parse_token>
      {
         if (auto m = ctre::match<keywords_re_1>(expr))
         {
            return parse_token{.type = token_type::keyword, .str = std::string(m.get<0>())};
         }
         else if (auto m = ctre::match<keywords_re_2>(expr))
         {
            return parse_token{.type = token_type::keyword, .str = std::string(m.get<0>())};
         }
         else if (auto m = ctre::match<keywords_re_3>(expr))
         {
            return parse_token{.type = token_type::keyword, .str = std::string(m.get<0>())};
         }
         else
         {
            return std::nullopt;
         }
      }

      auto re_match_number(std::string_view expr) -> std::optional<parse_token>
      {
         if (auto m = ctre::match<int_decimal_re>(expr))
         {
            return parse_token{.type = token_type::number, .str = std::string(m.get<0>())};
         }
         else if (auto m = ctre::match<int_hex_re>(expr))
         {
            return parse_token{.type = token_type::number, .str = std::string(m.get<0>())};
         }
         else if (auto m = ctre::match<int_octal_re>(expr))
         {
            return parse_token{.type = token_type::number, .str = std::string(m.get<0>())};
         }
         else if (auto m = ctre::match<int_binary_re>(expr))
         {
            return parse_token{.type = token_type::number, .str = std::string(m.get<0>())};
         }
         else if (auto m = ctre::match<float_hex_re>(expr))
         {
            return parse_token{.type = token_type::number, .str = std::string(m.get<0>())};
         }
         else
         {
            return std::nullopt;
         }
      }

      auto re_match_named_literal(const std::string_view expr) -> std::optional<parse_token>
      {
         if (auto m = ctre::match<named_literal_re>(expr))
         {
            return parse_token{.type = token_type::named_literal, .str = std::string(m.get<0>())};
         }
         else
         {
            return std::nullopt;
         }
      }

      auto re_match(std::string_view expr) -> std::optional<parse_token>
      {
         // TODO(wmbat): Use C++23's monadic interface for optional
         if (auto m = re_match_whitespace(expr))
         {
            return m;
         }
         else if (auto m = re_match_identifier(expr))
         {
            return m;
         }
         else if (auto m = re_match_string(expr))
         {
            return m;
         }
         else if (auto m = re_match_named_literal(expr))
         {
            return m;
         }
         else if (auto m = re_match_number(expr))
         {
            return m;
         }
         else if (auto m = re_match_keyword(expr))
         {
            return m;
         }
         else
         {
            return std::nullopt;
         }
      }
   } // namespace detail
} // namespace gerbil::inline v0
