/**
 * @file dense_hash_map.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date 17th of July, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "util/containers/detail/growth_policy.hpp"
#include "util/containers/detail/node.hpp"
#include "util/containers/dynamic_array.hpp"
#include "util/iterators/random_access_iterator.hpp"

#include <concepts>
#include <iterator>

namespace util
{
   template <std::totally_ordered key_, class any_, size_t buffer_size_,
      class compare_ = std::less<key_>, class policy_ = detail::growth_policy::power_of_two>
   class small_dense_avl_tree
   {
      using node_type = detail::node<key_, any_>;
      using node_index_type = typename node_type::index_t;
      using nodes_container_type = small_dynamic_array<node_type, buffer_size_, policy_>;
      using nodes_size_type = typename nodes_container_type::size_type;
      using buckets_container_type = small_dynamic_array<nodes_size_type, buffer_size_, policy_>;

   public:
      using key_type = key_;
      using mapped_type = any_;
      using value_type = std::pair<const key_type, mapped_type>;
      using size_type = nodes_size_type;
      using difference_type = typename nodes_container_type::difference_type;
      using key_compare = compare_;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = value_type*;
      using const_pointer = value_type*;
      using iterator = random_access_iterator<value_type>;
      using const_iterator = random_access_iterator<const value_type>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;
   };
} // namespace util
