
#include <algorithm>
#include <cstdint>
#include <limits>

namespace util // NOLINT
{
   namespace detail::growth_policy
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
         static constexpr auto compute_new_capacity(std::size_t min_capacity) -> std::size_t
         {
            constexpr auto max_capacity = std::size_t{1}
               << (std::numeric_limits<std::size_t>::digits - 1);

            if (min_capacity > max_capacity)
            {
               return max_capacity;
            }

            --min_capacity;

            for (auto i = 1; i < std::numeric_limits<std::size_t>::digits; i *= 2)
            {
               min_capacity |= min_capacity >> i;
            }

            return ++min_capacity;
         }
      };
   } // namespace detail::growth_policy
} // namespace util
