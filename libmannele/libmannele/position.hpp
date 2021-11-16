
#ifndef LIBMANNELE_POSITION_HPP_
#define LIBMANNELE_POSITION_HPP_

#include <libmannele/core.hpp>
#include <libmannele/maths/number.hpp>

namespace mannele
{
   template <number Type>
   struct position
   {
      using value_type = Type;

      value_type x;
      value_type y;
   };

   using position_f32 = position<f32>;
   using position_f64 = position<f64>;

   using position_i16 = position<i16>;
   using position_i32 = position<i32>;
   using position_i64 = position<i64>;

   using position_u16 = position<u16>;
   using position_u32 = position<u32>;
   using position_u64 = position<u64>;
} // namespace mannele

#endif // LIBMANNELE_POSITION_HPP_
