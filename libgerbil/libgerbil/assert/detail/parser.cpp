#include <libgerbil/assert/detail/parser.hpp>

#include <algorithm>
#include <array>

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

      // https://eel.is/c++draft/gram.lex
      //

      consteval auto get_sorted_keywords()
      {
         constexpr std::size_t keyword_count = 78;

         auto keywords = std::array<std::string_view, keyword_count>(
            {"alignas",    "constinit",    "public",        "alignof",
             "const_cast", "float",        "register",      "try",
             "asm",        "continue",     "for",           "reinterpret_cast",
             "typedef",    "auto",         "co_await",      "friend",
             "requires",   "typeid",       "bool",          "co_return",
             "goto",       "return",       "typename",      "break",
             "co_yield",   "if",           "short",         "union",
             "case",       "decltype",     "inline",        "signed",
             "unsigned",   "catch",        "default",       "int",
             "sizeof",     "using",        "char",          "delete",
             "long",       "static",       "virtual",       "char8_t",
             "do",         "mutable",      "static_assert", "void",
             "char16_t",   "double",       "namespace",     "static_cast",
             "volatile",   "char32_t",     "dynamic_cast",  "new",
             "struct",     "wchar_t",      "class",         "else",
             "noexcept",   "switch",       "while",         "concept",
             "enum",       "template",     "const",         "explicit",
             "operator",   "this",         "consteval",     "export",
             "private",    "thread_local", "constexpr",     "extern",
             "protected",  "throw"});

         const auto cmp = [](const std::string_view a, const std::string_view b) {
            if (a.length() > b.length())
               return true;
            else if (a.length() == b.length())
               return a < b;
            else
               return false;
         };

         std::sort(std::begin(keywords), std::end(keywords), cmp);

         return keywords;
      }
      consteval auto get_sorted_punctuators()
      {
         constexpr std::size_t punct_count = 66;

         auto punctuators = std::array<std::string_view, punct_count>{
            "{",     "}",      "[",     "]",      "(",      ")",  "<:",  ":>",  "<%",     "%>",
            ";",     ":",      "...",   "?",      "::",     ".",  ".*",  "->",  "->*",    "~",
            "!",     "+",      "-",     "*",      "/",      "%",  "^",   "&",   "|",      "=",
            "+=",    "-=",     "*=",    "/=",     "%=",     "^=", "&=",  "|=",  "==",     "!=",
            "<",     ">",      "<=",    ">=",     "<=>",    "&&", "||",  "<<",  ">>",     "<<=",
            ">>=",   "++",     "--",    ",",      "and",    "or", "xor", "not", "bitand", "bitor",
            "compl", "and_eq", "or_eq", "xor_eq", "not_eq", "#"};

         const auto cmp = [](const std::string_view a, const std::string_view b) {
            if (a.length() > b.length())
               return true;
            else if (a.length() == b.length())
               return a < b;
            else
               return false;
         };

         std::sort(std::begin(punctuators), std::end(punctuators), cmp);

         return punctuators;
      }

      [[maybe_unused]] constexpr auto keywords = get_sorted_keywords();
      [[maybe_unused]] constexpr auto punctuators = get_sorted_punctuators();
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
