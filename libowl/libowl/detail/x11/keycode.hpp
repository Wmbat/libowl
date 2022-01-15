#ifndef LIBOWL_DETAIL_X11_KEYCODE_HPP_
#define LIBOWL_DETAIL_X11_KEYCODE_HPP_

#include <libowl/types.hpp>

#include <compare>
#include <limits>

namespace owl::inline v0
{
   namespace x11
   {
      /**
       * @brief Strongly typed struct holding an X server keycode as a u32
       */
      class keycode_t
      {
      public:
         /**
          * @brief Default construct the class. It will hold a u32 value equal to the max value a 32
          * bit unsigned integer may hold.
          */
         constexpr keycode_t() = default;
         /**
          * @brief Explicitely construct class from a u32
          *
          * @param[in] value The value to be stored
          */
         explicit constexpr keycode_t(u8 value) : m_value(value) {}

         /**
          * @brief Access the local value.
          */
         constexpr auto value() noexcept -> u8& { return m_value; }
         /**
          * @brief Access the local value.
          */
         [[nodiscard]] constexpr auto value() const noexcept -> u32 { return m_value; }

         constexpr auto operator<=>(const keycode_t& other) const -> std::strong_ordering = default;
         constexpr auto operator==(const keycode_t& other) const -> bool = default;

      private:
         u8 m_value = std::numeric_limits<u8>::max();
      };

   } // namespace x11
} // namespace owl::inline v0

#endif // LIBOWL_DETAIL_X11_KEYCODE_HPP_
