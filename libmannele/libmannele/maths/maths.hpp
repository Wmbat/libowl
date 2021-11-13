/**
 * @file libmannele/maths/maths.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 26th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBMANNELE_MATHS_MATHS_HPP_
#define LIBMANNELE_MATHS_MATHS_HPP_

#include <libmannele/maths/number.hpp>

namespace mannele
{
   template <number Any>
   constexpr auto fast_pow(Any num, unsigned int pow) -> Any
   {
      // NOLINTNEXTLINE
      return (pow >= sizeof(unsigned int) * 8) ? 0 : pow == 0 ? 1 : num * fast_pow(num, pow - 1);
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

} // namespace mannele

#endif // LIBMANNELE_MATHS_MATHS_HPP_
