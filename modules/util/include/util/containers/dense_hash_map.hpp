
#include <util/containers/detail/growth_policy.hpp>
#include <util/containers/dynamic_array.hpp>

#include <functional>

namespace util
{
   template <class key_, class any_, size_t buffer_size_, class hash_ = std::hash<key_>,
      class pred_ = std::equal_to<key_>, class policy_ = detail::growth_policy::golden_ratio>
   class small_dense_hash_map
   {
      using policy_::compute_new_capacity;
   };

   template <class key_, class any_, class hash_ = std::hash<key_>,
      class pred_ = std::equal_to<key_>, class policy_ = detail::growth_policy::golden_ratio>
   using dense_hash_map = small_dense_hash_map<key_, any_, 0, hash_, pred_, policy_>;
} // namespace util
