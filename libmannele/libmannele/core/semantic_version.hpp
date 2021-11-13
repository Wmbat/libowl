#ifndef LIBMANNELE_CORE_SEMANTIC_VERSION_HPP_
#define LIBMANNELE_CORE_SEMANTIC_VERSION_HPP_

#include <libmannele/core/types.hpp>

#include <compare>

namespace mannele
{
   struct semantic_version
   {
      u32 major;
      u32 minor;
      u32 patch;

      friend auto operator==(const semantic_version& lhs, const semantic_version& rhs)
         -> bool = default;
      friend auto operator<=>(const semantic_version& lhs, const semantic_version& rhs)
         -> std::strong_ordering = default;
   };

} // namespace mannele

#endif // LIBMANNELE_CORE_SEMANTIC_VERSION_HPP_
