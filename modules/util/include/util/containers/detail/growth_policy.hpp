/**
 * @file growth_policy.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date 17th of July, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>

namespace util::detail::growth_policy
{
   inline static constexpr float GOLDEN_RATIO = 1.618f;

   struct golden_ratio
   {
      static constexpr auto compute_new_capacity(std::size_t min_capacity) -> std::size_t
      {
         return std::min(static_cast<std::size_t>(min_capacity * GOLDEN_RATIO),
            std::numeric_limits<std::size_t>::max());
      }
   };

   struct power_of_two
   {
   };
} // namespace util::detail::growth_policy
