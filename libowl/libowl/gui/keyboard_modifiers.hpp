/**
 *
 */

#ifndef LIBOWL_GUI_KEYBOARD_MODIFIERS_HPP_
#define LIBOWL_GUI_KEYBOARD_MODIFIERS_HPP_

#include <libowl/types.hpp>

#include <libmannele/core/flags.hpp>

#include <fmt/core.h>

namespace owl::inline v0
{
   /**
    * @brief Possible keyboard key modifiers
    */
   enum struct key_modifiers_flag_bits : u32
   {
      shift = 0x001,
      caps_lock = 0x002,
      ctrl = 0x004,
      mod_1 = 0x008,
      mod_2 = 0x010,
      mod_3 = 0x020,
      mod_4 = 0x040,
      mod_5 = 0x080,
   };

   /**
    * @brief A flag type that operates on modifier_flag_bits
    */
   using key_modifier_flags = mannele::flags<key_modifiers_flag_bits>;

   constexpr auto operator|(key_modifiers_flag_bits bit0, key_modifiers_flag_bits bit1) noexcept
      -> key_modifier_flags
   {
      return key_modifier_flags(bit0) | bit1;
   }
   constexpr auto operator&(key_modifiers_flag_bits bit0, key_modifiers_flag_bits bit1) noexcept
      -> key_modifier_flags
   {
      return key_modifier_flags(bit0) & bit1;
   }
} // namespace owl::inline v0

template <>
struct mannele::flag_traits<owl::key_modifiers_flag_bits>
{
private:
   using bits = owl::key_modifiers_flag_bits;
   using mask_type = owl::key_modifier_flags::mask_type;

public:
   enum : owl::u32
   {
      all_flags = static_cast<mask_type>(bits::shift | bits::caps_lock | bits::ctrl | bits::mod_1
                                         | bits::mod_2 | bits::mod_3 | bits::mod_4 | bits::mod_5)
   };
};

template <>
struct fmt::formatter<owl::key_modifier_flags>
{
   constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
   {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const owl::key_modifier_flags& flags, FormatContext& ctx) -> decltype(ctx.out())
   {
      using owl::key_modifiers_flag_bits;

      std::string str;
      if (flags & key_modifiers_flag_bits::shift)
      {
         str += "Shift";
      }

      if (flags & key_modifiers_flag_bits::caps_lock)
      {
         str += str.empty() ? "CapsLock" : "+CapsLock";
      }

      if (flags & key_modifiers_flag_bits::ctrl)
      {
         str += str.empty() ? "Ctrl" : "+Ctrl";
      }

      return format_to(ctx.out(), "{}", str);
   }
};

#endif // LIBOWL_GUI_KEYBOARD_MODIFIERS_HPP_
