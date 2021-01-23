#pragma once

#include <type_traits>

namespace cacao
{
   namespace detail
   {
      template <typename>
      struct is_scoped_enum : std::false_type
      {
      };

      // clang-format off
      template <typename T> requires std::is_enum_v<T> 
      struct is_scoped_enum<T> :
         std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>>
      {
      };
      // clang-format on

      template <typename Any>
      inline constexpr bool is_scoped_enum_v = is_scoped_enum<Any>::value;
   } // namespace detail

   template <typename FlagBitsType>
   struct flag_traits
   {
      enum
      {
         all = 0
      };
   };

   // clang-format off

   template <typename BitType> requires detail::is_scoped_enum_v<BitType>
   class flags
   {
   public:
      using bit_type = BitType;
      using mask_type = typename std::underlying_type<BitType>::type;

      constexpr flags() = default;
      constexpr flags(BitType bit) noexcept : m_mask(static_cast<mask_type>(bit)) {}
      constexpr explicit flags(mask_type flags) noexcept : m_mask(flags) {}

      constexpr auto operator<=>(const flags&) const = default;

      constexpr auto operator!() const noexcept -> bool { return !m_mask; }

      constexpr auto operator&(const flags& rhs) const noexcept -> flags
      {
         return flags{m_mask & rhs.m_mask};
      }
      constexpr auto operator|(const flags& rhs) const noexcept -> flags
      {
         return flags{m_mask | rhs.m_mask};
      }
      constexpr auto operator^(const flags& rhs) const noexcept -> flags
      {
         return flags{m_mask ^ rhs.m_mask};
      }
      constexpr auto operator~() const noexcept -> flags
      {
         return flags{m_mask ^ flag_traits<BitType>::all};
      }

      constexpr auto operator|=(const flags& rhs) noexcept -> flags&
      {
         m_mask |= rhs.m_mask;
         return *this;
      }
      constexpr auto operator&=(const flags& rhs) noexcept -> flags&
      {
         m_mask &= rhs.m_mask;
         return *this;
      }
      constexpr auto operator^=(const flags& rhs) noexcept -> flags&
      {
         m_mask ^= rhs.m_mask;
         return *this;
      }

      explicit constexpr operator bool() const noexcept { return !!m_mask; }
      explicit constexpr operator mask_type() const noexcept { return m_mask; }

   private:
      mask_type m_mask{0};
   };
   // clang-format on

   template <typename BitType>
   constexpr auto operator&(BitType bit, const ::cacao::flags<BitType>& flags) noexcept
      -> ::cacao::flags<BitType>
   {
      return flags & bit;
   }

   template <typename BitType>
   constexpr auto operator|(BitType bit, const ::cacao::flags<BitType>& flags) noexcept
      -> ::cacao::flags<BitType>
   {
      return flags | bit;
   }

   template <typename BitType>
   constexpr auto operator^(BitType bit, const ::cacao::flags<BitType>& flags) noexcept
      -> ::cacao::flags<BitType>
   {
      return flags ^ bit;
   }
} // namespace cacao

#define DEFINE_EXTRA_ENUM_OPERATORS(flag_type)                                                     \
   constexpr auto operator|(flag_type::bit_type bit0,                                              \
                            flag_type::bit_type bit1) noexcept->queue_flags                        \
   {                                                                                               \
      return flag_type(bit0) | bit1;                                                               \
   }                                                                                               \
   constexpr auto operator&(flag_type::bit_type bit0,                                              \
                            flag_type::bit_type bit1) noexcept->queue_flags                        \
   {                                                                                               \
      return flag_type(bit0) & bit1;                                                               \
   }                                                                                               \
   constexpr auto operator~(flag_type::bit_type bits) noexcept->queue_flags                        \
   {                                                                                               \
      return ~(flag_type(bits));                                                                   \
   }                                                                                               \
   \
