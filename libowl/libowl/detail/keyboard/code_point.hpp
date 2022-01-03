#ifndef LIBOWL_DETAIL_KEYBOARD_UCS_CODE_HPP_
#define LIBOWL_DETAIL_KEYBOARD_UCS_CODE_HPP_

#include <libowl/types.hpp>

#include <compare>

namespace owl::inline v0
{
   namespace detail
   {
      class code_point_t
      {
      public:
         constexpr code_point_t() = default;
         explicit constexpr code_point_t(u32 value) : m_value(value) {}

         constexpr auto value() noexcept -> u32& { return m_value; }
         [[nodiscard]] constexpr auto value() const noexcept -> u32 { return m_value; }

         constexpr auto operator<=>(const code_point_t& other) const
            -> std::strong_ordering = default;
         constexpr auto operator==(const code_point_t& other) const -> bool = default;

      private:
         u32 m_value;
      };
   } // namespace detail
} // namespace owl::inline v0

#endif // LIBOWL_DETAIL_KEYBOARD_UCS_CODE_HPP_
