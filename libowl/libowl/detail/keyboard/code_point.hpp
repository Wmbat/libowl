/**
 * @file libowl/detail/keyboard/code_point.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2022 wmbat.
 */

#ifndef LIBOWL_DETAIL_KEYBOARD_CODE_POINT_HPP_
#define LIBOWL_DETAIL_KEYBOARD_CODE_POINT_HPP_

#include <libowl/types.hpp>

#include <compare>
#include <limits>

namespace owl::inline v0
{
   namespace detail
   {
      /**
       * @brief Strongly typed struct holding a unicode code point as a 32 bit unsigned integer
       */
      class code_point_t
      {
      public:
         /**
          * @brief Default construct the class. It will hold a u32 value equal to the max value a 32
          * bit unsigned integer may hold.
          */
         constexpr code_point_t() = default;
         /**
          * @brief Explicitely construct class from a u32
          *
          * @param[in] value The value to be stored
          */
         explicit constexpr code_point_t(u32 value) : m_value(value) {}

         /**
          * @brief Access the local value.
          */
         constexpr auto value() noexcept -> u32& { return m_value; }
         /**
          * @brief Access the local value.
          */
         [[nodiscard]] constexpr auto value() const noexcept -> u32 { return m_value; }

         constexpr auto operator<=>(const code_point_t& other) const
            -> std::strong_ordering = default;
         constexpr auto operator==(const code_point_t& other) const -> bool = default;

      private:
         u32 m_value = std::numeric_limits<u32>::max();
      };
   } // namespace detail
} // namespace owl::inline v0

#endif // LIBOWL_DETAIL_KEYBOARD_CODE_POINT_HPP_
