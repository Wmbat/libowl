#ifndef LIBMANNELE_DIMENSION_HPP
#define LIBMANNELE_DIMENSION_HPP

#include <libmannele/concepts.hpp>
#include <libmannele/core.hpp>

namespace mannele
{
   template <number Type>
   struct dimension
   {
      using value_type = Type;
        
      value_type width;
      value_type height;
   };

   using dimension_f32 = dimension<f32>;
   using dimension_f64 = dimension<f64>;

   using dimension_i32 = dimension<i32>;
   using dimension_i64 = dimension<i64>;

   using dimension_u32 = dimension<u32>;
   using dimension_u64 = dimension<u64>;
} // namespace mannele

#endif // LIBMANNELE_DIMENSION_HPP
