#include <libgerbil/assert/detail/parser.hpp>

#if not defined(let)
#   define let const auto
#endif

namespace gerbil::inline v0
{
   namespace
   {
      static constexpr char digit_zero = 0x0030;
      static constexpr char digit_one = 0x0031;
      static constexpr char lowest_digit = digit_zero;
      static constexpr char highest_digit = 0x0039;

      static constexpr char latin_capital_b = 0x0042;
      static constexpr char latin_small_b = 0x0062;

      static constexpr char latin_capital_x = 0x0058;
      static constexpr char latin_small_x = 0x0078;

      auto is_non_zero_digit(char c) noexcept -> bool
      {
         return c >= digit_one && c <= highest_digit;
      }
      auto is_digit(char c) noexcept -> bool { return c >= lowest_digit && c <= highest_digit; }

      auto is_binary_token(char c) noexcept -> bool
      {
         return c == latin_capital_b || c == latin_small_b;
      }
      auto is_binary_prefix(char first, char second) -> bool
      {
         return first == digit_zero && is_binary_token(second);
      }

      auto is_hexadecimal_token(char c) noexcept -> bool
      {
         return c == latin_capital_x || c == latin_small_x;
      }
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
      }
   } // namespace detail
} // namespace gerbil::inline v0
