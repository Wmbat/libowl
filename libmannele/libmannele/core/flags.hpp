#ifndef LIBMANNELE_CORE_FLAGS_HPP_
#define LIBMANNELE_CORE_FLAGS_HPP_

#include <compare>
#include <type_traits>

namespace mannele
{
   template <typename T>
   concept scoped_enum = std::is_scoped_enum_v<T>;

   template <typename BitType>
   struct flag_traits
   {
      enum
      {
         all_flags = 0
      };
   };

   template <scoped_enum T>
   class flags
   {
   public:
      using flag_type = T;
      using mask_type = std::underlying_type_t<T>;

   public:
      constexpr flags() = default;
      constexpr flags(flag_type bit) noexcept : m_mask(static_cast<mask_type>(bit)) {} // NOLINT
      constexpr explicit flags(mask_type flags) noexcept : m_mask(flags) {}

      explicit constexpr operator bool() const noexcept { return !!m_mask; }
      explicit constexpr operator mask_type() const noexcept { return m_mask; }

      constexpr auto operator<=>(const flags&) const = default;

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
         return flags{m_mask ^ flag_traits<flag_type>::all_flags};
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

   private:
      mask_type m_mask{0};
   };

   template <scoped_enum BitType>
   constexpr auto operator&(BitType bit, const flags<BitType>& flag) noexcept -> flags<BitType>
   {
      return flag & bit;
   }

   template <scoped_enum BitType>
   constexpr auto operator|(BitType bit, const flags<BitType>& flag) noexcept -> flags<BitType>
   {
      return flag | bit;
   }

   template <scoped_enum BitType>
   constexpr auto operator^(BitType bit, const flags<BitType>& flag) noexcept -> flags<BitType>
   {
      return flag ^ bit;
   }

// NOLINTNEXTLINE
#define MANNELE_DEFINE_EXTRA_ENUM_OPERATORS(flag)                                                  \
   constexpr auto operator|(flag::flag_type bit0, flag::flag_type bit1) noexcept->flag             \
   {                                                                                               \
      return flag(bit0) | bit1;                                                                    \
   }                                                                                               \
   constexpr auto operator&(flag::flag_type bit0, flag::flag_type bit1) noexcept->flag             \
   {                                                                                               \
      return flag(bit0) & bit1;                                                                    \
   }                                                                                               \
   constexpr auto operator~(flag::flag_type bits) noexcept->flag { return ~(flag(bits)); }

} // namespace mannele

#endif // LIBMANNELE_CORE_FLAGS_HPP_
