#ifndef LIBOWL_TYPES_HPP_
#define LIBOWL_TYPES_HPP_

#include <cstdint>

namespace owl::inline v0
{
   using i8 = std::int8_t;   ///< 8 bit signed integer type
   using i16 = std::int16_t; ///< 16 bit signed integer type
   using i32 = std::int32_t; ///< 32 bit signed integer type
   using i64 = std::int64_t; ///< 64 bit signed integer type

   using u8 = std::uint8_t;
   using u16 = std::uint16_t;
   using u32 = std::uint32_t;
   using u64 = std::uint64_t;

   using f32 = float;
   using f64 = double;
} // namespace owl::inline v0

#endif // LIBOWL_TYPES_HPP_
