#ifndef LIBMANNELE_ALGORITHM_BINARY_SEARCH_HPP_
#define LIBMANNELE_ALGORITHM_BINARY_SEARCH_HPP_

#include <range/v3/algorithm/lower_bound.hpp>
#include <range/v3/range/dangling.hpp>

#include <functional>

namespace mannele::inline v0
{
   template <
      std::forward_iterator I, std::sentinel_for<I> S, typename T, typename Proj = ranges::identity,
      ranges::indirect_strict_weak_order<const T*, ranges::projected<I, Proj>> Comp = ranges::less>
   auto binary_search(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) -> I
   {
      first = ranges::lower_bound(first, last, value, comp, proj);
      return first != last && !comp(value, proj(*first)) ? first : last;
   }

   template <
      typename R, typename T, typename Proj = ranges::identity,
      ranges::indirect_strict_weak_order<const T*, ranges::projected<ranges::iterator_t<R>, Proj>>
         Comp = ranges::less>
   auto binary_search(R&& r, const T& value, Comp comp = {}, Proj proj = {})
      -> ranges::borrowed_iterator_t<R>
   {
      const auto first = ranges::lower_bound(r, value, comp, proj);
      const auto last = ranges::end(r);

      return first != last && !comp(value, proj(*first)) ? first : last;
   }
} // namespace mannele::inline v0

#endif // LIBMANNELE_ALGORITHM_BINARY_SEARCH_HPP_
