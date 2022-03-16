#ifndef LIBASH_DETAIL_TALLY_RATING_HPP_
#define LIBASH_DETAIL_TALLY_RATING_HPP_

#include <libash/core.hpp>

namespace ash::inline v0
{
   namespace detail
   {
      /**
       * @brief Compute the sum of a variadic set of integer values.
       *
       * @param[in] values A set of integers
       *
       * @return The sum of all integer values passed as parameter or -1 if any is equal to -1.
       */
      constexpr auto tally_ratings(std::signed_integral auto... values) -> decltype((... + values))
      {
         if ((... || (-1 == values)))
         {
            return -1;
         }
         else
         {
            return (... + values);
         }
      }
   } // namespace detail
} // namespace ash::inline v0

#endif // LIBASH_DETAIL_TALLY_RATING_HPP_
