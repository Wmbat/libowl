#ifndef LIBMANNELE_CORE_SEMANTIC_VERSION_HPP_
#define LIBMANNELE_CORE_SEMANTIC_VERSION_HPP_

#include <libmannele/core/types.hpp>

namespace mannele
{
   struct semantic_version
   {
      u32 major;
      u32 minor;
      u32 patch;
   };
} // namespace mannele

#endif // LIBMANNELE_CORE_SEMANTIC_VERSION_HPP_
