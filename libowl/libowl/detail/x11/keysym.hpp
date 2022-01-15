#ifndef LIBOWL_DETAIL_X11_KEYSYM_HPP_
#define LIBOWL_DETAIL_X11_KEYSYM_HPP_

#include <libowl/detail/keyboard/code_point.hpp>
#include <libowl/detail/x11/connection.hpp>
#include <libowl/detail/x11/keycode.hpp>
#include <libowl/gui/keyboard_modifiers.hpp>
#include <libowl/types.hpp>

#include <libreglisse/maybe.hpp>

#include <assert.hpp>

#include <compare>
#include <limits>

namespace owl::inline v0
{
   namespace x11::detail
   {
      auto compute_keysim_offset(key_modifier_flags flags) -> u32;
   }

   namespace x11
   {

      /**
       * @brief Strongly typed struct holding an X server keysym as a u32
       */
      class keysym_t
      {
      public:
         /**
          * @brief Default construct the class. It will hold a u32 value equal to the max value a 32
          * bit unsigned integer may hold.
          */
         constexpr keysym_t() = default;
         /**
          * @brief Explicitely construct class from a u32
          *
          * @param[in] value The value to be stored
          */
         explicit constexpr keysym_t(u32 value) : m_value(value) {}
         /**
          * @brief Construct a keysym from a keycode and its modifiers
          *
          * @param[in] conn The connection to the X server
          * @param[in] keycode The keycode received in the key press event
          * @param[in] mods The modifiers associated with the keycode
          *
          * @pre \p keycode must be greater than or equal to \p conn.min_keycode
          * @pre \p keycode must be smaller than or equal to \p conn.max_keycode
          */
         constexpr keysym_t(const connection& conn, keycode_t keycode, key_modifier_flags mods)
         {
            assert(keycode.value() >= conn.min_keycode); // NOLINT
            assert(keycode.value() <= conn.max_keycode); // NOLINT

            const auto keysyms_per_key_code = conn.keysyms_per_keycode;
            const auto adjusted_key_code = keycode.value() - conn.min_keycode;
            const auto keysym_offset = detail::compute_keysim_offset(mods);

            m_value = conn.keysyms[keysym_offset + adjusted_key_code * keysyms_per_key_code];
         }

         /**
          * @brief Access the local value.
          */
         constexpr auto value() noexcept -> u32& { return m_value; }
         /**
          * @brief Access the local value.
          */
         [[nodiscard]] constexpr auto value() const noexcept -> u32 { return m_value; }

         constexpr auto operator<=>(const keysym_t& other) const -> std::strong_ordering = default;
         constexpr auto operator==(const keysym_t& other) const -> bool = default;

      private:
         u32 m_value;
      };

      /**
       * @brief
       *
       * @param[in] keysym
       *
       * @return
       */
      auto to_code_point(keysym_t keysym) -> reglisse::maybe<owl::detail::code_point_t>;
   } // namespace x11
} // namespace owl::inline v0

#endif // LIBOWL_DETAIL_X11_KEYSYM_HPP_
