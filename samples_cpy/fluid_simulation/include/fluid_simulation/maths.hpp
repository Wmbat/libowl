#pragma once

#include <concepts>

// clang-format off
template <typename Any>
concept number = std::integral<Any> || std::floating_point<Any>;
// clang-format on

template <number Any>
constexpr auto my_pow(Any num, unsigned int pow) -> Any
{
   // NOLINTNEXTLINE
   return (pow >= sizeof(unsigned int) * 8) ? 0 : pow == 0 ? 1 : num * my_pow(num, pow - 1);
}

template <number Any>
constexpr auto half(Any num) -> Any
{
   return num / static_cast<Any>(2);
}

template <number Any>
constexpr auto square(Any num) -> Any
{
   return num * num;
}

template <number Any>
constexpr auto cube(Any num) -> Any
{
   return num * num * num;
};

template <typename Num>
auto reciprocal(Num num) -> Num
{
   return static_cast<Num>(1) / num;
}

template <typename Num>
auto is_negative(Num num) -> bool
{
   return num < static_cast<Num>(1);
}
