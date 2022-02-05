#include <libowl/detail/x11/keysym.hpp>

#include <libowl/detail/x11/keysym_to_code_point_table.hpp>

#include <libmannele/algorithm/binary_search.hpp>

using reglisse::maybe;
using reglisse::none;
using reglisse::some;

namespace owl::inline v0
{
   namespace x11
   {
      auto to_code_point(keysym_t keysym) -> maybe<owl::detail::code_point_t>
      {
         // TODO(wmbat): We may be able to optimize this for standard ASCII characters

         const auto it = mannele::binary_search(keysym_to_code_point_table, keysym, std::ranges::less(),
                                                [](const auto& p) {
                                                   return p.keysym;
                                                });

         if (it != std::ranges::end(keysym_to_code_point_table))
         {
            return some(it->code_point);
         }
         else
         {
            return none;
         }
      }
   } // namespace x11
} // namespace owl::inline v0
