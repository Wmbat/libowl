#ifndef LIBGERBIL_ASSERT_HPP_
#define LIBGERBIL_ASSERT_HPP_

// Jeremy Rifkin 2021
// https://github.com/jeremy-rifkin/asserts
//
// Adapted by wmbat

#include <libgerbil/assert/detail/parser.hpp>
#include <libgerbil/assert/detail/source_location.hpp>

#include <fmt/core.h>

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <limits>
#include <optional>
#include <set>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#if defined(__clang__)
#   define IS_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
#   define IS_GCC 1
#else
#   error "no"
#endif
#if defined(_WIN32)
#   define IS_WINDOWS 1
#   ifndef STDIN_FILENO
#      define STDIN_FILENO _fileno(stdin)
#      define STDOUT_FILENO _fileno(stdout)
#      define STDERR_FILENO _fileno(stderr)
#   endif
#elif defined(__linux)
#   define IS_LINUX 1
#   include <unistd.h>
#else
#   error "no"
#endif

// if defined by a previous #include...
#ifdef assert
#   undef assert
#endif

#ifndef NCOLOR
#   define ESC "\033["
#   define ANSIRGB(r, g, b) ESC "38;2;" #   r ";" #   g ";" #   b "m"
#   define RESET ESC "0m"
// Slightly modified one dark pro colors
// Original: https://coolors.co/e06b74-d19a66-e5c07a-98c379-62aeef-55b6c2-c678dd
// Modified: https://coolors.co/e06b74-d19a66-e5c07a-90cc66-62aeef-56c2c0-c678dd
#   define RED ANSIRGB(224, 107, 116)
#   define ORANGE ANSIRGB(209, 154, 102)
#   define YELLOW ANSIRGB(229, 192, 122)
#   define GREEN ANSIRGB(150, 205, 112) // modified
#   define BLUE ANSIRGB(98, 174, 239)
#   define CYAN ANSIRGB(86, 194, 192) // modified
#   define PURPL ANSIRGB(198, 120, 221)
#else
#   define RED ""
#   define ORANGE ""
#   define YELLOW ""
#   define GREEN ""
#   define BLUE ""
#   define CYAN ""
#   define PURPL ""
#   define RESET ""
#endif

#ifdef ASSERT_FAIL_ACTION
extern void ASSERT_FAIL_ACTION();
#else
#   define ASSERT_FAIL_ACTION fail // NOLINT
#endif

// NOLINTNEXTLINE
#define PHONY_USE(E)                                                                               \
   do                                                                                              \
   {                                                                                               \
      using x [[maybe_unused]] = decltype(E);                                                      \
   } while (0)

namespace gerbil::inline v0
{
   namespace detail
   {
      static constexpr size_t indent_size = 8;

      void primitive_assert_impl(bool c, bool verification, const char* expression,
                                 const char* message = nullptr, source_location location = {});

#ifdef ASSERT_INTERNAL_DEBUG
#   define primitive_assert(c, ...) primitive_assert_impl(c, false, #   c, ##__VA_ARGS__)
#else
#   define primitive_assert(c, ...) PHONY_USE(c) // NOLINT
#endif

// Still present in release mode, nonfatal
#define internal_verify(c, ...) primitive_assert_impl(c, true, #c, ##__VA_ARGS__) // NOLINT

      /*
       * string utilities
       */

      template <typename... T>
      [[gnu::cold]] auto stringf(T... args) -> std::string
      {
         int length = snprintf(0, 0, args...);
         if (length < 0)
         {
            primitive_assert(false, "Invalid arguments to stringf");
         }

         std::string str(static_cast<std::size_t>(length), 0);
         snprintf(str.data(), static_cast<size_t>(length) + 1u, args...);
         return str;
      }

      auto indent(const std::string_view str, size_t depth, char c = ' ', bool ignore_first = false)
         -> std::string;

      /*
       * system wrappers
       */

      void enable_virtual_terminal_processing_if_needed();

#ifdef _WIN32
      typedef int pid_t;
#endif

      auto getpid() -> pid_t;

      void wait_for_keypress();

      auto isatty(int fd) -> bool;

      // https://stackoverflow.com/questions/23369503/get-size-of-terminal-window-rows-columns
      auto terminal_width() -> int;

      auto strerror_wrapper(int err) -> std::string; // stupid C stuff, stupid microsoft stuff

      /*
       * Stacktrace implementation
       */

      // All i the .cpp

      /*
       * Expression decomposition
       */

      struct nothing
      {
      };

      template <typename T>
      constexpr bool is_nothing = std::is_same_v<T, nothing>;

      // Hack to get around static_assert(false); being evaluated before any instantiation, even
      // under an if-constexpr branch
      template <typename T>
      constexpr bool always_false = false;

      template <typename T>
      using strip = std::remove_cv_t<std::remove_reference_t<T>>;

      template <typename A, typename B>
      constexpr bool isa = std::is_same_v<strip<A>, B>; // intentionally not stripping B

      // Is integral but not boolean
      template <typename T>
      constexpr bool is_integral_notb = std::is_integral_v<strip<T>> && !isa<T, bool>;

      // Lots of boilerplate
      // Using int comparison functions here to support proper signed comparisons. Need to make sure
      // assert(map.count(1) == 2) doesn't produce a warning. It wouldn't under normal circumstances
      // but it would in this library due to the parameters being forwarded down a long chain.
      // And we want to provide as much robustness as possible anyways.
      // Copied and pasted from https://en.cppreference.com/w/cpp/utility/intcmp
      // Not using std:: versions because library is targetting C++17
      template <typename T, typename U>
      [[gnu::cold]] constexpr auto cmp_equal(T t, U u) -> bool
      {
         using UT = std::make_unsigned_t<T>;
         using UU = std::make_unsigned_t<U>;
         if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
            return t == u;
         else if constexpr (std::is_signed_v<T>)
            return t < 0 ? false : UT(t) == u;
         else
            return u < 0 ? false : t == UU(u);
      }

      template <typename T, typename U>
      [[gnu::cold]] constexpr auto cmp_not_equal(T t, U u) -> bool
      {
         return !cmp_equal(t, u);
      }

      template <typename T, typename U>
      [[gnu::cold]] constexpr auto cmp_less(T t, U u) -> bool
      {
         using UT = std::make_unsigned_t<T>;
         using UU = std::make_unsigned_t<U>;
         if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
            return t < u;
         else if constexpr (std::is_signed_v<T>)
            return t < 0 ? true : UT(t) < u;
         else
            return u < 0 ? false : t < UU(u);
      }

      template <typename T, typename U>
      [[gnu::cold]] constexpr auto cmp_greater(T t, U u) -> bool
      {
         return cmp_less(u, t);
      }

      template <typename T, typename U>
      [[gnu::cold]] constexpr auto cmp_less_equal(T t, U u) -> bool
      {
         return !cmp_greater(t, u);
      }

      template <typename T, typename U>
      [[gnu::cold]] constexpr auto cmp_greater_equal(T t, U u) -> bool
      {
         return !cmp_less(t, u);
      }

      // Lots of boilerplate
      // std:: implementations don't allow two separate types for lhs/rhs
      // Note: is this macro potentially bad when it comes to debugging(?)
      namespace ops
      {
// NOLINTNEXTLINE
#define gen_op_boilerplate(name, op)                                                               \
   struct name                                                                                     \
   {                                                                                               \
      static constexpr std::string_view op_string = #op;                                           \
      template <typename A, typename B>                                                            \
      [[gnu::cold]] constexpr auto operator()(A&& lhs, B&& rhs) const                              \
      { /* no need to forward ints */                                                              \
         return std::forward<A>(lhs) op std::forward<B>(rhs);                                      \
      }                                                                                            \
   }
// NOLINTNEXTLINE
#define gen_op_boilerplate_special(name, op, cmp)                                                  \
   struct name                                                                                     \
   {                                                                                               \
      static constexpr std::string_view op_string = #op;                                           \
      template <typename A, typename B>                                                            \
      [[gnu::cold]] constexpr auto operator()(A&& lhs, B&& rhs) const                              \
      { /* no need to forward ints */                                                              \
         if constexpr (is_integral_notb<A> && is_integral_notb<B>)                                 \
            return cmp(lhs, rhs);                                                                  \
         else                                                                                      \
            return std::forward<A>(lhs) op std::forward<B>(rhs);                                   \
      }                                                                                            \
   }
         gen_op_boilerplate(shl, <<);
         gen_op_boilerplate(shr, >>);
         gen_op_boilerplate_special(eq, ==, cmp_equal);
         gen_op_boilerplate_special(neq, !=, cmp_not_equal);
         gen_op_boilerplate_special(gt, >, cmp_greater);
         gen_op_boilerplate_special(lt, <, cmp_less);
         gen_op_boilerplate_special(gteq, >=, cmp_greater_equal);
         gen_op_boilerplate_special(lteq, <=, cmp_less_equal);
         gen_op_boilerplate(band, &);
         gen_op_boilerplate(bxor, ^);
         gen_op_boilerplate(bor, |);
         gen_op_boilerplate(land, &&);
         gen_op_boilerplate(lor, ||);
         gen_op_boilerplate(assign, =);
         gen_op_boilerplate(add_assign, +=);
         gen_op_boilerplate(sub_assign, -=);
         gen_op_boilerplate(mul_assign, *=);
         gen_op_boilerplate(div_assign, /=);
         gen_op_boilerplate(mod_assign, %=);
         gen_op_boilerplate(shl_assign, <<=);
         gen_op_boilerplate(shr_assign, >>=);
         gen_op_boilerplate(band_assign, &=);
         gen_op_boilerplate(bxor_assign, ^=);
         gen_op_boilerplate(bor_assign, |=);
#undef gen_op_boilerplate
      } // namespace ops

      // I learned this automatic expression decomposition trick from lest:
      // https://github.com/martinmoene/lest/blob/master/include/lest/lest.hpp#L829-L853
      //
      // I have improved upon the trick by supporting more operators and adding perfect forwarding.
      //
      // Some cases to test and consider:
      //
      // Expression   Parsed as
      // Basic:
      // false        << false
      // a == b       (<< a) == b
      //
      // Equal precedence following:
      // a << b       (<< a) << b
      // a << b << c  ((<< a) << b) << c
      // a << b + c   (<< a) << (b + c)
      // a << b < c   ((<< a) << b) < c  // edge case
      //
      // Higher precedence following:
      // a + b        << (a + b)
      // a + b + c    << ((a + b) + c)
      // a + b * c    << (a + (b * c))
      // a + b < c    (<< (a + b)) < c
      //
      // Lower precedence following:
      // a < b        (<< a) < b
      // a < b < c    ((<< a) < b) < c
      // a < b + c    (<< a) < (b + c)
      // a < b == c   ((<< a) < b) == c // edge case

      template <typename A = nothing, typename B = nothing, typename C = nothing>
      struct expression_decomposer
      {
         A a;
         B b;
         [[gnu::cold]] explicit expression_decomposer() = default;
         template <typename U>
         [[gnu::cold]] explicit expression_decomposer(U&& a) : a(std::forward<U>(a))
         {}
         template <typename U, typename V>
         [[gnu::cold]] explicit expression_decomposer(U&& a, V&& b) :
            a(std::forward<U>(a)), b(std::forward<V>(b))
         {}
         // Note: get_value or operator bool() should only be invoked once
         [[gnu::cold]] auto get_value()
         {
            if constexpr (is_nothing<C>)
            {
               static_assert(is_nothing<B> && !is_nothing<A>);
               return std::forward<A>(a);
            }
            else
            {
               return C()(std::forward<A>(a), std::forward<B>(b));
            }
         }
         operator bool() { return get_value(); }
         // Need overloads for operators with precedence <= bitshift
         // TODO: spaceship operator?
         // Note: Could decompose more than just comparison and boolean operators, but it would take
         // a lot of work and I don't think it's beneficial for this library.
         template <typename O>
         [[gnu::cold]] auto operator<<(O&& operand)
         {
            using Q =
               std::conditional_t<std::is_rvalue_reference_v<O>, std::remove_reference_t<O>, O>;
            if constexpr (is_nothing<A>)
            {
               static_assert(is_nothing<B> && is_nothing<C>);
               return expression_decomposer<Q, nothing, nothing>(std::forward<O>(operand));
            }
            else if constexpr (is_nothing<B>)
            {
               static_assert(is_nothing<C>);
               return expression_decomposer<A, Q, ops::shl>(std::forward<A>(a),
                                                            std::forward<O>(operand));
            }
            else
            {
               static_assert(!is_nothing<C>);
               return expression_decomposer<decltype(get_value()), O, ops::shl>(
                  std::forward<A>(get_value()), std::forward<O>(operand));
            }
         }
// NOLINTNEXTLINE
#define gen_op_boilerplate(functor, op)                                                            \
   template <typename O>                                                                           \
   [[gnu::cold]] auto operator op(O&& operand)                                                     \
   {                                                                                               \
      static_assert(!is_nothing<A>);                                                               \
      using Q = std::conditional_t<std::is_rvalue_reference_v<O>, std::remove_reference_t<O>, O>;  \
      if constexpr (is_nothing<B>)                                                                 \
      {                                                                                            \
         static_assert(is_nothing<C>);                                                             \
         return expression_decomposer<A, Q, functor>(std::forward<A>(a),                           \
                                                     std::forward<O>(operand));                    \
      }                                                                                            \
      else                                                                                         \
      {                                                                                            \
         static_assert(!is_nothing<C>);                                                            \
         return expression_decomposer<decltype(get_value()), Q, functor>(                          \
            std::forward<A>(get_value()), std::forward<O>(operand));                               \
      }                                                                                            \
   }
         gen_op_boilerplate(ops::shr, >>);
         gen_op_boilerplate(ops::eq, ==);
         gen_op_boilerplate(ops::neq, !=);
         gen_op_boilerplate(ops::gt, >);
         gen_op_boilerplate(ops::lt, <);
         gen_op_boilerplate(ops::gteq, >=);
         gen_op_boilerplate(ops::lteq, <=);
         gen_op_boilerplate(ops::band, &);
         gen_op_boilerplate(ops::bxor, ^);
         gen_op_boilerplate(ops::bor, |);
         gen_op_boilerplate(ops::land, &&);
         gen_op_boilerplate(ops::lor, ||);
         gen_op_boilerplate(ops::assign, =);
         gen_op_boilerplate(ops::add_assign, +=);
         gen_op_boilerplate(ops::sub_assign, -=);
         gen_op_boilerplate(ops::mul_assign, *=);
         gen_op_boilerplate(ops::div_assign, /=);
         gen_op_boilerplate(ops::mod_assign, %=);
         gen_op_boilerplate(ops::shl_assign, <<=);
         gen_op_boilerplate(ops::shr_assign, >>=);
         gen_op_boilerplate(ops::band_assign, &=);
         gen_op_boilerplate(ops::bxor_assign, ^=);
         gen_op_boilerplate(ops::bor_assign, |=);
#undef gen_op_boilerplate
      };

      // for ternary support
      template <typename U>
      expression_decomposer(U&&) -> expression_decomposer<
         std::conditional_t<std::is_rvalue_reference_v<U>, std::remove_reference_t<U>, U>>;

      /*
       * C++ syntax analysis logic
       */

      struct highlight_block
      {
         std::string_view color;
         std::string content;
      };

      auto prettify_type(std::string type) -> std::string;

      auto highlight(const std::string& expression) -> std::string;

      auto highlight_blocks(const std::string& expression) -> std::vector<highlight_block>;

      auto trim_suffix(const std::string& expression) -> std::string;

      auto is_bitwise(std::string_view op) -> bool;

      auto decompose_expression(const std::string& expression, const std::string_view target_op)
         -> std::pair<std::string, std::string>;

      /*
       * stringification
       */

      auto escape_string(const std::string_view str, char quote) -> std::string;

      template <typename T>
      [[gnu::cold]] constexpr auto type_name() -> std::string_view
      {
         // Cases to handle:
         // gcc:   constexpr std::string_view ns::type_name() [with T = int; std::string_view =
         // std::basic_string_view<char>] clang: std::string_view ns::type_name() [T = int]
         auto substring_bounded_by = [](std::string_view sig, std::string_view l,
                                        std::string_view r) {
            primitive_assert(sig.find(l) != std::string_view::npos);
            primitive_assert(sig.rfind(r) != std::string_view::npos);
            primitive_assert(sig.find(l) < sig.rfind(r));
            auto i = sig.find(l) + l.length();
            return sig.substr(i, sig.rfind(r) - i);
         };
#if IS_CLANG
         return substring_bounded_by(__PRETTY_FUNCTION__, "[T = ", "]");
#elif IS_GCC
         return substring_bounded_by(__PRETTY_FUNCTION__, "[with T = ", "; std::string_view = ");
#endif
      }

      // https://stackoverflow.com/questions/28309164/checking-for-existence-of-an-overloaded-member-function
      template <typename T>
      class has_stream_overload
      {
         template <typename C,
                   typename = decltype(std::declval<std::ostringstream>() << std::declval<C>())>
         static auto test(int) -> std::true_type;
         template <typename C>
         static auto test(...) -> std::false_type;

      public:
         static constexpr bool value = decltype(test<T>(0))::value;
      };

      template <typename T>
      constexpr bool has_stream_overload_v = has_stream_overload<T>::value;

      template <typename T>
      constexpr bool is_string_type =
         isa<
            T,
            std::
               string> || isa<T, std::string_view> || isa<std::decay_t<strip<T>>, char*> // <-
                                                                                         // covers
                                                                                         // literals
                                                                                         // (i.e.
                                                                                         // const
                                                                                         // char(&)[N])
                                                                                         // too
         || isa<std::decay_t<strip<T>>, const char*>;

      // test cases
      static_assert(is_string_type<char*>);
      static_assert(is_string_type<const char*>);
      static_assert(is_string_type<char[5]>);
      static_assert(is_string_type<const char[5]>);
      static_assert(!is_string_type<char (*)[5]>);
      static_assert(is_string_type<char (&)[5]>);
      static_assert(is_string_type<const char (&)[27]>);
      static_assert(!is_string_type<std::vector<char>>);
      static_assert(!is_string_type<int>);
      static_assert(is_string_type<std::string>);
      static_assert(is_string_type<std::string_view>);

      template <typename T>
      [[gnu::cold]] auto stringify(const T& t,
                                   [[maybe_unused]] std::optional<literal_type> fmt = std::nullopt)
         -> std::string
      {
         // bool and char need to be before std::is_integral
         if constexpr (is_string_type<T>)
         {
            if constexpr (std::is_pointer_v<strip<T>>)
            {
               if (t == nullptr)
               {
                  return "nullptr";
               }
            }
            // TODO: re-evaluate this...? What if just comparing two buffer pointers?
            return escape_string(t, '"'); // string view may need explicit construction?
         }
         else if constexpr (isa<T, char>)
         {
            return escape_string({&t, 1}, '\'');
         }
         else if constexpr (isa<T, bool>)
         {
            return t ? "true" : "false"; // streams/boolalpha not needed for this
         }
         else if constexpr (isa<T, std::nullptr_t>)
         {
            return "nullptr";
         }
         else if constexpr (std::is_pointer_v<strip<T>>)
         {
            if (t == nullptr)
            { // weird nullptr shenanigans, only prints "nullptr" for nullptr_t
               return "nullptr";
            }
            else
            {
               std::ostringstream oss;
               // Manually format the pointer - ostream::operator<<(void*) falls back to %p which
               // is implementation-defined. MSVC prints pointers without the leading "0x" which
               // messes up the highlighter.
               oss << std::showbase << std::hex << uintptr_t(t);
               return std::move(oss).str();
            }
         }
         else if constexpr (std::is_integral_v<T>)
         {
            if (fmt)
            {
               if (fmt == literal_type::binary)
               {
                  return fmt::format("{:#0b}", t);
               }
               else if (fmt == literal_type::octal)
               {
                  return fmt::format("{:#0}", t);
               }
               else if (fmt == literal_type::decimal)
               {
                  return fmt::format("{}", t);
               }
               else
               {
                  return fmt::format("{:#0x}", t);
               }
            }
            else
            {
               primitive_assert(false, "unexpected literal format requested for printing");
               return "";
            }
         }
         else if constexpr (std::is_floating_point_v<T>)
         {
            if (fmt)
            {
               if (fmt == literal_type::decimal)
               {
                  return fmt::format("{}", t);
               }
               else if (fmt == literal_type::hexadecimal)
               {
                  return fmt::format("{:#0x}", t);
               }
               else
               {
                  primitive_assert(false, "unexpected literal format requested for printing");
                  return "";
               }
            }
            else
            {
               primitive_assert(false, "unexpected literal format requested for printing");
               return "";
            }
         }
         else
         {
            if constexpr (has_stream_overload_v<T>)
            {
               std::ostringstream oss;
               oss << t;
               return std::move(oss).str();
            }
            else
            {
               return stringf("<instance of %s>",
                              prettify_type(std::string(type_name<T>())).c_str());
            }
         }
      }

      /*
       * stack trace printing
       */

      struct column_t
      {
         size_t width;
         std::vector<highlight_block> blocks;
         bool right_align = false;
      };

      void wrapped_print(const std::vector<column_t>& columns);

      void print_stacktrace();

      /*
       * binary diagnostic printing
       */

      auto gen_assert_binary(bool verify, const std::string& a_str, const char* op,
                             const std::string& b_str, size_t n_vargs) -> std::string;

      template <typename T>
      [[gnu::cold]] auto generate_stringifications(T&& v, const std::set<literal_type>& formats)
         -> std::vector<std::string>
      {
         if constexpr (std::is_arithmetic<strip<T>>::value && !isa<T, bool> && !isa<T, char>)
         {
            std::vector<std::string> vec;
            for (literal_type fmt : formats)
            {
               // TODO: consider pushing empty fillers to keep columns aligned later on? Does not
               // matter at the moment because floats only have decimal and hex literals but could
               // if more formats are added.
               std::string str = stringify(v, fmt);
               if (!str.empty())
               {
                  vec.push_back(std::move(str));
               }
            }

            return vec;
         }
         else
         {
            return {stringify(v)};
         }
      }

      void print_values(const std::vector<std::string>& vec, size_t lw);

      auto get_values(const std::vector<std::string>& vec) -> std::vector<highlight_block>;

      constexpr int min_term_width = 50;

      template <typename A, typename B>
      [[gnu::cold]] void print_binary_diagnostic(A&& a, B&& b, const char* a_str, const char* b_str,
                                                 std::string_view op)
      {
         // std::set used so formats are printed in a specific order
         std::set<literal_type> formats = {literal_type::decimal};
         if (auto lformat = parse_literal_type(a_str))
         {
            formats.insert(*lformat);
         }
         if (auto rformat = parse_literal_type(b_str))
         {
            formats.insert(*rformat);
         }

         if (is_bitwise(op))
         {
            formats.insert(literal_type::binary); // always display binary for bitwise
         }
         primitive_assert(formats.size() > 0);

         // generate raw strings for given formats, without highlighting
         std::vector<std::string> lstrings = generate_stringifications(std::forward<A>(a), formats);
         std::vector<std::string> rstrings = generate_stringifications(std::forward<B>(b), formats);
         primitive_assert(lstrings.size() > 0);
         primitive_assert(rstrings.size() > 0);

         // pad all columns where there is overlap
         // TODO: Use column printer instead of manual padding.
         for (size_t i = 0; i < std::min(lstrings.size(), rstrings.size()); i++)
         {
            // find which clause, left or right, we're padding (entry i)
            std::vector<std::string>& which =
               lstrings[i].length() < rstrings[i].length() ? lstrings : rstrings;
            int difference = std::abs((int)lstrings[i].length() - (int)rstrings[i].length());
            if (i != which.size() - 1)
            { // last column excluded as padding is not necessary at the end of the line
               which[i].insert(which[i].end(), static_cast<std::size_t>(difference), ' ');
            }
         }
         // determine whether we actually gain anything from printing a where clause (e.g. exclude
         // "1
         // => 1")
         struct
         {
            bool left, right;
         } has_useful_where_clause = {
            formats.size() > 1 || lstrings.size() > 1
               || (a_str != lstrings[0] && trim_suffix(a_str) != lstrings[0]),
            formats.size() > 1 || rstrings.size() > 1
               || (b_str != rstrings[0] && trim_suffix(b_str) != rstrings[0])};
         // print where clause
         if (has_useful_where_clause.left || has_useful_where_clause.right)
         {
            size_t lw = std::max(has_useful_where_clause.left ? strlen(a_str) : 0,
                                 has_useful_where_clause.right ? strlen(b_str) : 0);
            const auto term_width =
               static_cast<std::size_t>(terminal_width()); // will be 0 on error
            // Limit lw to about half the screen. TODO: Re-evaluate what we want to do here.
            if (term_width > 0)
            {
               lw = std::min(lw, term_width / 2 - indent_size - 4 /* arrow */);
            }

            fmt::print(stderr, "    Where:\n");
            auto print_clause = [term_width, lw](const char* expr_str,
                                                 std::vector<std::string>& expr_strs) {
               if (term_width >= min_term_width)
               {
                  wrapped_print(
                     {{indent_size - 1, {{"", ""}}},
                      {lw, highlight_blocks(expr_str)},
                      {2, {{"", "=>"}}},
                      {term_width - lw - indent_size - 4 /* arrow */, get_values(expr_strs)}});
               }
               else
               {
                  fprintf(stderr, "        %s%*s => ", highlight(expr_str).c_str(),
                          int(lw - strlen(expr_str)), "");
                  print_values(expr_strs, lw);
               }
            };
            if (has_useful_where_clause.left)
            {
               print_clause(a_str, lstrings);
            }
            if (has_useful_where_clause.right)
            {
               print_clause(b_str, rstrings);
            }
         }
      }

      /*
       * actual assertion main bodies, finally
       */

      // allow non-fatal assertions
      enum class ASSERT
      {
         NONFATAL,
         FATAL
      };

      void fail();

#define X(x) #x   // NOLINT
#define Y(x) X(x) // NOLINT
      constexpr const std::string_view errno_expansion = Y(errno);
#undef Y
#undef X

      struct extra_diagnostics
      {
         std::optional<ASSERT> fatality;
         std::string message;
         std::vector<std::pair<std::string, std::string>> entries;
         extra_diagnostics();
         extra_diagnostics(const extra_diagnostics&);
         extra_diagnostics(extra_diagnostics&&) noexcept;
         ~extra_diagnostics();
         auto operator=(const extra_diagnostics&) -> extra_diagnostics&;
         auto operator=(extra_diagnostics&&) noexcept -> extra_diagnostics&;
         auto operator+(const extra_diagnostics& other) -> extra_diagnostics&;
      };

      template <size_t I = 0, size_t N>
      [[gnu::cold]] void process_args_step(extra_diagnostics&, const char* const (&)[N])
      {}

      template <size_t I = 0, size_t N, typename T, typename... Args>
      [[gnu::cold]] void process_args_step(extra_diagnostics& entry,
                                           const char* const (&args_strings)[N], T& t,
                                           Args&... args)
      {
         if constexpr (isa<T, ASSERT>)
         {
            entry.fatality = t;
         }
         else if constexpr (I == 0 && is_string_type<T>)
         {
            entry.message = t;
         }
         else
         {
            // two cases to handle: errno and regular diagnostics
            if (isa<T, strip<decltype(errno)>> && args_strings[I] == errno_expansion)
            {
               // this is redundant and useless but the body for errno handling needs to be in an
               // if constexpr wrapper
               if constexpr (isa<T, strip<decltype(errno)>>)
               {
                  // errno will expand to something hideous like (*__errno_location()),
                  // may as well replace it with "errno"
                  entry.entries.push_back(
                     {"errno", stringf("%2d \"%s\"", t, strerror_wrapper(t).c_str())});
               }
            }
            else
            {
               entry.entries.push_back({args_strings[I], stringify(t, literal_type::decimal)});
            }
         }
         process_args_step<I + 1>(entry, args_strings, args...);
      }

      template <size_t I = 0, size_t N, typename... Args>
      [[gnu::cold]] auto process_args(const char* const (&args_strings)[N], Args&... args)
         -> extra_diagnostics
      {
         extra_diagnostics entry;
         process_args_step(entry, args_strings, args...);
         return entry;
      }

      struct lock
      {
         lock();
         ~lock();
         lock(const lock&) = delete;
         lock(lock&&) = delete;
         auto operator=(const lock&) -> lock& = delete;
         auto operator=(lock&&) -> lock& = delete;
      };

      template <size_t N, typename... Args>
      [[gnu::cold]] void assert_fail_generic(bool verify, const char* pretty_func,
                                             source_location location,
                                             const std::function<void()>& assert_printer,
                                             const std::string& assert_string,
                                             const char* const (&args_strings)[N], Args&... args)
      {
         lock l;
         static_assert((sizeof...(args) == 0 && N == 2) || N == sizeof...(args) + 1);
         auto [fatal, message, extra_diagnostics] = process_args(args_strings, args...);
         enable_virtual_terminal_processing_if_needed();
         const char* action = verify ? "Verification" : "Assertion";
         if (message != "")
         {
            fmt::print(stderr, "{} failed at {}:{}: {}: {}\n", action, location.file_name(),
                       location.line(), pretty_func, message);
         }
         else
         {
            fmt::print(stderr, "{} failed at {}:{}: {}:\n", action, location.file_name(),
                       location.line(), pretty_func);
         }
         fprintf(stderr, "    %s\n", highlight(assert_string).c_str());
         assert_printer();
         if (!extra_diagnostics.empty())
         {
            fmt::print(stderr, "    Extra diagnostics:\n");
            auto term_width = static_cast<size_t>(terminal_width()); // will be 0 on error
            size_t lw = 0;
            for (auto& entry : extra_diagnostics)
            {
               lw = std::max(lw, entry.first.size());
            }
            for (auto& entry : extra_diagnostics)
            {
               if (term_width >= min_term_width)
               {
                  wrapped_print({{indent_size - 1, {{"", ""}}},
                                 {lw, highlight_blocks(entry.first)},
                                 {2, {{"", "=>"}}},
                                 {term_width - lw - indent_size - 4 /* arrow */,
                                  highlight_blocks(entry.second)}});
               }
               else
               {
                  fprintf(stderr, "        %s%*s => %s\n", highlight(entry.first).c_str(),
                          int(lw - entry.first.length()), "",
                          indent(highlight(entry.second), indent_size + lw + 4, ' ', true).c_str());
               }
            }
         }
         fmt::print(stderr, "\nStack trace:\n");
         print_stacktrace();
         if (fatal == ASSERT::FATAL)
         {
            ASSERT_FAIL_ACTION();
         }
#ifdef _0_ASSERT_DEMO
         fprintf(stderr, "\n");
#endif
      }

      template <typename A, typename B, typename C, size_t N, typename... Args>
      [[gnu::cold]] [[gnu::noinline]] void
      assert_fail(expression_decomposer<A, B, C>& decomposer, const char* expr_str, bool verify,
                  const char* pretty_func, source_location location,
                  const char* const (&args_strings)[N], Args&... args)
      {
         assert_fail_generic(
            verify, pretty_func, location,
            [&]() {
               if constexpr (is_nothing<C>)
               {
                  static_assert(is_nothing<B> && !is_nothing<A>);
               }
               else
               {
                  auto [a_str, b_str] = decompose_expression(expr_str, C::op_string);
                  print_binary_diagnostic(std::forward<A>(decomposer.a),
                                          std::forward<B>(decomposer.b), a_str.c_str(),
                                          b_str.c_str(), C::op_string);
               }
            },
            stringf("%s(%s%s);", verify ? "verify" : "assert", expr_str,
                    sizeof...(args) > 0 ? ", ..." : ""),
            args_strings, args...);
      }

      template <typename C, typename A, typename B, size_t N, typename... Args>
      [[gnu::cold]] [[gnu::noinline]] void
      assert_binary_fail(A&& a, B&& b, const char* a_str, const char* b_str, const char* raw_op,
                         bool verify, const char* pretty_func, source_location location,
                         const char* const (&args_strings)[N], Args&... args)
      {
         assert_fail_generic(
            verify, pretty_func, location,
            [&a, &b, a_str, b_str]() {
               print_binary_diagnostic(std::forward<A>(a), std::forward<B>(b), a_str, b_str,
                                       C::op_string);
            },
            gen_assert_binary(verify, a_str, raw_op, b_str, sizeof...(args)), args_strings,
            args...);
      }

      // top-level assert functions emplaced by the macros
      // these are the only non-cold functions

      template <typename A, typename B, typename C, size_t N, typename... Args>
      void assert_decomposed(expression_decomposer<A, B, C> decomposer, const char* expr_str,
                             const char* pretty_func, source_location location,
                             const char* const (&args_strings)[N], Args&&... args)
      {
         if (!(bool)decomposer.get_value())
         {
            enable_virtual_terminal_processing_if_needed();
            // todo: forward decomposer?
            assert_fail(decomposer, expr_str, false, highlight(pretty_func).c_str(), location,
                        args_strings, args...);
         }
         else
         {
#ifdef _0_ASSERT_DEMO
            assert_decomposed(expression_decomposer(false), "false", __PRETTY_FUNCTION__, {},
                              {"", ""}, "assert should have failed");
#endif
         }
      }

      template <typename C, typename A, typename B, size_t N, typename... Args>
      void assert_binary(A&& a, B&& b, const char* a_str, const char* b_str, const char* raw_op,
                         const char* pretty_func, source_location location,
                         const char* const (&args_strings)[N], Args&&... args)
      {
         if (!(bool)C()(a, b))
         {
            enable_virtual_terminal_processing_if_needed();
            assert_binary_fail<C>(std::forward<A>(a), std::forward<B>(b), a_str, b_str, raw_op,
                                  false, highlight(pretty_func).c_str(), location, args_strings,
                                  args...);
         }
         else
         {
#ifdef _0_ASSERT_DEMO
            assert_decomposed(expression_decomposer(false), "false", __PRETTY_FUNCTION__, {},
                              {"", ""}, "assert should have failed");
#endif
         }
      }

      template <typename A, typename B, typename C, size_t N, typename... Args>
      auto verify_decomposed(expression_decomposer<A, B, C> decomposer, const char* expr_str,
                             const char* pretty_func, source_location location,
                             const char* const (&args_strings)[N], Args&&... args)
      {
         auto x = decomposer.get_value();
         if (!(bool)x)
         {
            enable_virtual_terminal_processing_if_needed();
            // todo: forward decomposer?
            assert_fail(decomposer, expr_str, true, highlight(pretty_func).c_str(), location,
                        args_strings, args...);
         }
         else
         {
#ifdef _0_ASSERT_DEMO
            assert_decomposed(expression_decomposer(false), "false", __PRETTY_FUNCTION__, {},
                              {"", ""}, "assert should have failed");
#endif
         }
         return x;
      }

      template <typename C, typename A, typename B, size_t N, typename... Args>
      auto verify_binary(A&& a, B&& b, const char* a_str, const char* b_str, const char* raw_op,
                         const char* pretty_func, source_location location,
                         const char* const (&args_strings)[N], Args&&... args)
      {
         auto x = C()(a, b);
         if (!(bool)x)
         {
            enable_virtual_terminal_processing_if_needed();
            assert_binary_fail<C>(std::forward<A>(a), std::forward<B>(b), a_str, b_str, raw_op,
                                  true, highlight(pretty_func).c_str(), location, args_strings,
                                  args...);
         }
         else
         {
#ifdef _0_ASSERT_DEMO
            assert_decomposed(expression_decomposer(false), "false", __PRETTY_FUNCTION__, {},
                              {"", ""}, "assert should have failed");
#endif
         }
         return x;
      }

#ifndef _0_ASSERT_CPP // keep macros for the .cpp
#   undef primitive_assert
#   undef internal_verify
#endif
   } // namespace detail
} // namespace gerbil::inline v0

#ifndef _0_ASSERT_CPP // keep macros for the .cpp
#   ifndef NCOLOR
#      undef ESC
#      undef ANSIRGB
#   endif
#   undef RED
#   undef ORANGE
#   undef YELLOW
#   undef GREEN
#   undef BLUE
#   undef CYAN
#   undef PURPL
#   undef RESET
#endif

using gerbil::detail::ASSERT;

// NOLINTBEGIN

// Macro mapping utility by William Swanson
// https://github.com/swansontec/map-macro/blob/master/map.h
#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,
#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next) MAP_NEXT1(MAP_GET_END test, next)
#define MAP0(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP1)(f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP0)(f, peek, __VA_ARGS__)
#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next) MAP_LIST_NEXT1(MAP_GET_END test, next)
#define MAP_LIST0(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST1)(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST0)(f, peek, __VA_ARGS__)
#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
#define ASSERT_DETAIL_STRINGIFY(x) #x,
// __PRETTY_FUNCTION__ used because __builtin_FUNCTION() used in source_location (like __FUNCTION__)
// is just the method name, not signature
#if IS_GCC
// __VA_OPT__ needed for GCC, https://gcc.gnu.org/bugzilla/show_bug.cgi?id=44317
#   define ASSERT_DETAIL_ARGS(...)                                                                 \
      __PRETTY_FUNCTION__, {}, /* extra string here because of extra comma from map */             \
         {MAP(ASSERT_DETAIL_STRINGIFY, ##__VA_ARGS__) ""} __VA_OPT__(, )##__VA_ARGS__
#else
// clang properly eats the comma with ##__VA_ARGS__
#   define ASSERT_DETAIL_ARGS(...)                                                                 \
      __PRETTY_FUNCTION__, {}, /* extra string here because of extra comma from map */             \
         {MAP(ASSERT_DETAIL_STRINGIFY, ##__VA_ARGS__) ""}, ##__VA_ARGS__
#endif

#ifdef NDEBUG
#   ifndef ASSUME_ASSERTS
#      define ASSERT(expr, ...) (void)0
#      define ASSERT_EQ(a, b, ...) (void)0
#      define ASSERT_NEQ(a, b, ...) (void)0
#      define ASSERT_LT(a, b, ...) (void)0
#      define ASSERT_GT(a, b, ...) (void)0
#      define ASSERT_LTEQ(a, b, ...) (void)0
#      define ASSERT_GTEQ(a, b, ...) (void)0
#      define ASSERT_AND(a, b, ...) (void)0
#      define ASSERT_OR(a, b, ...) (void)0
#   else
// Note: This is not a default because Sometimes assertion expressions have side effects that are
// undesirable at runtime in an `NDEBUG` build like exceptions on std container operations which
// cannot be optimized away. These often end up not being helpful hints either.
// TODO: Need to feed through decomposer to preserve sign-unsigned safety on top-level compare?
#      define ASSERT(expr, ...)                                                                    \
         do                                                                                        \
         {                                                                                         \
            if (!(expr))                                                                           \
               __builtin_unreachable();                                                            \
         } while (0)
#      define ASSERT_EQ(a, b, ...)                                                                 \
         do                                                                                        \
         {                                                                                         \
            if (!((a) == (b)))                                                                     \
               __builtin_unreachable();                                                            \
         } while (0)
#      define ASSERT_NEQ(a, b, ...)                                                                \
         do                                                                                        \
         {                                                                                         \
            if (!((a) != (b)))                                                                     \
               __builtin_unreachable();                                                            \
         } while (0)
#      define ASSERT_LT(a, b, ...)                                                                 \
         do                                                                                        \
         {                                                                                         \
            if (!((a) < (b)))                                                                      \
               __builtin_unreachable();                                                            \
         } while (0)
#      define ASSERT_GT(a, b, ...)                                                                 \
         do                                                                                        \
         {                                                                                         \
            if (!((a) > (b)))                                                                      \
               __builtin_unreachable();                                                            \
         } while (0)
#      define ASSERT_LTEQ(a, b, ...)                                                               \
         do                                                                                        \
         {                                                                                         \
            if (!((a) <= (b)))                                                                     \
               __builtin_unreachable();                                                            \
         } while (0)
#      define ASSERT_GTEQ(a, b, ...)                                                               \
         do                                                                                        \
         {                                                                                         \
            if (!((a) >= (b)))                                                                     \
               __builtin_unreachable();                                                            \
         } while (0)
#      define ASSERT_AND(a, b, ...)                                                                \
         do                                                                                        \
         {                                                                                         \
            if (!((a) && (b)))                                                                     \
               __builtin_unreachable();                                                            \
         } while (0)
#      define ASSERT_OR(a, b, ...)                                                                 \
         do                                                                                        \
         {                                                                                         \
            if (!((a) || (b)))                                                                     \
               __builtin_unreachable();                                                            \
         } while (0)
#   endif
#else
#   define ASSERT_DETAIL_GEN_CALL(a, b, op, ...)                                                   \
      gerbil::detail::assert_binary<gerbail::detail::ops::op>(a, b, #a, #b, #op,                   \
                                                              ASSERT_DETAIL_ARGS(__VA_ARGS__))
// detail::expression_decomposer(detail::expression_decomposer{}) done for ternary
// support
#   define ASSERT(expr, ...)                                                                       \
      _Pragma("GCC diagnostic ignored \"-Wparentheses\"") gerbil::detail::assert_decomposed(       \
         gerbil::detail::expression_decomposer(gerbil::detail::expression_decomposer{} << expr),   \
         #expr, ASSERT_DETAIL_ARGS(__VA_ARGS__))
#   define ASSERT_EQ(a, b, ...) ASSERT_DETAIL_GEN_CALL(a, b, eq, __VA_ARGS__)
#   define ASSERT_NEQ(a, b, ...) ASSERT_DETAIL_GEN_CALL(a, b, neq, __VA_ARGS__)
#   define ASSERT_LT(a, b, ...) ASSERT_DETAIL_GEN_CALL(a, b, lt, __VA_ARGS__)
#   define ASSERT_GT(a, b, ...) ASSERT_DETAIL_GEN_CALL(a, b, gt, __VA_ARGS__)
#   define ASSERT_LTEQ(a, b, ...) ASSERT_DETAIL_GEN_CALL(a, b, lteq, __VA_ARGS__)
#   define ASSERT_GTEQ(a, b, ...) ASSERT_DETAIL_GEN_CALL(a, b, gteq, __VA_ARGS__)
#   define ASSERT_AND(a, b, ...) ASSERT_DETAIL_GEN_CALL(a, b, land, __VA_ARGS__)
#   define ASSERT_OR(a, b, ...) ASSERT_DETAIL_GEN_CALL(a, b, lor, __VA_ARGS__)
#endif

#define ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, op, ...)                                               \
   gerbil::detail::verify_binary<gerbil::detail::ops::op>(a, b, #a, #b, #op,                       \
                                                          ASSERT_DETAIL_ARGS(__VA_ARGS__))
#if IS_GCC
#   define VERIFY(expr, ...)                                                                       \
      ({                                                                                           \
         _Pragma("GCC diagnostic ignored \"-Wparentheses\"")                                       \
            gerbil::detail::verify_decomposed(gerbil::detail::expression_decomposer(               \
                                                 gerbil::detail::expression_decomposer{} << expr), \
                                              #expr, ASSERT_DETAIL_ARGS(__VA_ARGS__));             \
      })
#else
#   define VERIFY(expr, ...)                                                                       \
      _Pragma("GCC diagnostic ignored \"-Wparentheses\"") gerbil::detail::verify_decomposed(       \
         gerbil::detail::expression_decomposer(gerbil::detail::expression_decomposer{} << expr),   \
         #expr, ASSERT_DETAIL_ARGS(__VA_ARGS__))
#endif
#define VERIFY_EQ(a, b, ...) ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, eq, __VA_ARGS__)
#define VERIFY_NEQ(a, b, ...) ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, neq, __VA_ARGS__)
#define VERIFY_LT(a, b, ...) ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, lt, __VA_ARGS__)
#define VERIFY_GT(a, b, ...) ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, gt, __VA_ARGS__)
#define VERIFY_LTEQ(a, b, ...) ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, lteq, __VA_ARGS__)
#define VERIFY_GTEQ(a, b, ...) ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, gteq, __VA_ARGS__)
#define VERIFY_AND(a, b, ...) ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, land, __VA_ARGS__)
#define VERIFY_OR(a, b, ...) ASSERT_DETAIL_GEN_VERIFY_CALL(a, b, lor, __VA_ARGS__)

// Aliases
#define assert(...) ASSERT(__VA_ARGS__) // provided for <assert.h> compatabilityn

// NOLINTEND

#endif // LIBGERBIL_ASSERT_HPP_
