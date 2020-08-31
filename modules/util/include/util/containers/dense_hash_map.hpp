/**
 * @file dense_hash_map.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date 17th of July, 2020.
 * @copyright MIT License.
 *
 * Follows closely this implementation of a dense hash map: https://github.com/Jiwan/dense_hash_map
 */

#pragma once

#include <util/containers/detail/disabling_structs.hpp>
#include <util/containers/dynamic_array.hpp>
#include <util/type_traits.hpp>

#include <functional>
#include <iterator>
#include <ranges>

namespace util
{
   namespace detail
   {
      template <class key_, class any_>
      union union_key_value_pair
      {
         using pair_t = std::pair<key_, any_>;
         using const_key_pair_t = std::pair<const key_, any_>;

         constexpr union_key_value_pair(auto&&... args) :
            pair_(std::forward<decltype(args)>(args)...)
         {}

         union_key_value_pair(std::allocator_arg_t, const allocator auto& alloc, auto&&... args)
         {
            auto alloc_copy = alloc;
            std::allocator_traits<decltype(alloc)>::construct(
               alloc_copy, &pair_, std::forward<decltype(args)>(args)...);
         }

         constexpr union_key_value_pair(const union_key_value_pair& other) noexcept(
            std::is_nothrow_copy_constructible_v<pair_t>) :
            pair_(other.pair_)
         {}

         constexpr union_key_value_pair(union_key_value_pair&& other) noexcept(
            std::is_nothrow_move_constructible_v<pair_t>) :
            pair_(std::move(other.pair_))
         {}

         // NOLINTNEXTLINE
         constexpr auto operator=(const union_key_value_pair& other) noexcept(
            std::is_nothrow_copy_assignable_v<pair_t>) -> union_key_value_pair&
         {
            if (this != &other)
            {
               pair_ = other.pair_;
            }
            return *this;
         }

         constexpr auto
         operator=(union_key_value_pair&& other) noexcept(std::is_nothrow_move_assignable_v<pair_t>)
            -> union_key_value_pair&
         {
            pair_ = std::move(other).pair_;
            return *this;
         }

         ~union_key_value_pair() { pair_.~pair_t(); }

         constexpr auto pair() -> pair_t& { return pair_; }
         constexpr auto pair() const -> const pair_t& { return pair_; }
         constexpr auto const_key_pair() -> const_key_pair_t& { return const_key_pair_; }
         constexpr auto const_key_pair() const -> const const_key_pair_t&
         {
            return const_key_pair_;
         }

      private:
         pair_t pair_;
         const_key_pair_t const_key_pair_;
      };

      template <class key_, class any_>
      struct safe_key_value_pair
      {
         using const_key_pair_t = std::pair<const key_, any_>;

         constexpr safe_key_value_pair(auto&&... args) :
            const_key_pair_(std::forward<decltype(args)>(args)...)
         {}
         constexpr safe_key_value_pair(std::allocator_arg_t, const allocator auto& alloc,
                                       auto&&... args)
         {
            auto alloc_copy = alloc;
            std::allocator_traits<decltype(alloc)>::construct(
               alloc_copy, &const_key_pair_, std::forward<decltype(args)>(args)...);
         }

         // Accessors to the two members.
         constexpr auto pair() -> const_key_pair_t& { return const_key_pair_; }
         constexpr auto pair() const -> const const_key_pair_t& { return const_key_pair_; }
         constexpr auto const_key_pair() -> const_key_pair_t& { return const_key_pair_; }
         constexpr auto const_key_pair() const -> const const_key_pair_t&
         {
            return const_key_pair_;
         }

      private:
         const_key_pair_t const_key_pair_;
      };

      template <class key_, class any_>
      inline constexpr bool are_pairs_standard_layout_v =
         std::is_standard_layout_v<std::pair<const key_, any_>>&&
            std::is_standard_layout_v<std::pair<key_, any_>>;

      template <class key_, class any_>
      using key_value_pair_t =
         std::conditional_t<are_pairs_standard_layout_v<key_, any_>,
                            union_key_value_pair<key_, any_>, safe_key_value_pair<key_, any_>>;

      template <class key_, class any_, class pair_ = std::pair<key_, any_>>
      struct node :
         disable_copy_constructor<pair_>,
         disable_copy_assignment<pair_>,
         disable_move_constructor<pair_>,
         disable_move_assignment<pair_>
      {
         using index_t = size_t;

         constexpr node(index_t next, auto&&... args) :
            next{next}, pair{std::forward<decltype(args)>(args)...}
         {}

         index_t next = std::numeric_limits<index_t>::max();
         key_value_pair_t<key_, any_> pair;
      };

      template <class hash_>
      using detect_transparent_key_equal = typename hash_::transparent_key_equal;

      template <class hash_>
      inline constexpr bool is_transparent_key_equal_v =
         is_detected<detect_transparent_key_equal, hash_>::value;

      template <class pred_>
      using detect_is_transparent = typename pred_::is_transparent;

      template <class pred_>
      inline constexpr bool is_transparent_v = is_detected<detect_is_transparent, pred_>::value;

      template <class Hash, class Pred, class Key, bool = is_transparent_key_equal_v<Hash>>
      struct key_equal
      {
         using type = Pred;
      };

      template <class Hash, class Pred, class Key>
      struct key_equal<Hash, Pred, Key, true>
      {
         using type = typename Hash::transparent_key_equal;

         static_assert(
            is_transparent_v<type>,
            "The associated transparent key equal is missing a is_transparent tag type.");
         static_assert(
            std::is_same_v<Pred, std::equal_to<Key>> || std::is_same_v<Pred, type>,
            "The associated transparent key equal must be the transparent_key_equal tag or "
            "std::equal_to<Key>");
      };

      static inline constexpr size_t min_bucket_container_size = 8u;

      consteval auto max_bucket_container_size(size_t size) noexcept -> size_t
      {
         return size >= min_bucket_container_size ? size : min_bucket_container_size;
      }

      //****************************************************//

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey>
      class bucket_iterator
      {
         using nodes_container_type = std::conditional_t<isConst, const Container, Container>;
         using node_index_type = typename node<Key, T>::index_t;
         using projected_type = std::pair<std::conditional_t<projectToConstKey, const Key, Key>, T>;

      public:
         using iterator_category = std::forward_iterator_tag;
         using value_type = std::conditional_t<isConst, const projected_type, projected_type>;
         using difference_type = std::ptrdiff_t;
         using reference = value_type&;
         using pointer = value_type*;

         constexpr bucket_iterator() = default;
         constexpr explicit bucket_iterator(nodes_container_type& nodes_container) :
            nodes_container(&nodes_container)
         {}

         constexpr bucket_iterator(node_index_type index, nodes_container_type& nodes_container) :
            nodes_container(&nodes_container), current_node_index_(index)
         {}

         constexpr auto operator*() const noexcept -> reference
         {
            if constexpr (projectToConstKey)
            {
               return (*nodes_container)[current_node_index_].pair.const_key_pair();
            }
            else
            {
               return (*nodes_container)[current_node_index_].pair.pair();
            }
         }

         constexpr auto operator++() noexcept -> bucket_iterator&
         {
            current_node_index_ = (*nodes_container)[current_node_index_].next;
            return *this;
         }

         constexpr auto operator++(int) noexcept -> bucket_iterator
         {
            auto old = (*this);
            ++(*this);
            return old;
         }

         constexpr auto operator->() const noexcept -> pointer
         {
            if constexpr (projectToConstKey)
            {
               return &(*nodes_container)[current_node_index_].pair.const_key_pair();
            }
            else
            {
               return &(*nodes_container)[current_node_index_].pair.pair();
            }
         }

         constexpr auto current_node_index() const -> node_index_type
         {
            return current_node_index_;
         }

      private:
         nodes_container_type* nodes_container;
         node_index_type current_node_index_ = std::numeric_limits<node_index_type>::max();
      };

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey,
                bool isConst2>
      constexpr auto operator==(
         const bucket_iterator<Key, T, Container, isConst, projectToConstKey>& lhs,
         const bucket_iterator<Key, T, Container, isConst2, projectToConstKey>& rhs) noexcept
         -> bool
      {
         return lhs.current_node_index() == rhs.current_node_index();
      }

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey,
                bool isConst2>
      constexpr auto operator!=(
         const bucket_iterator<Key, T, Container, isConst, projectToConstKey>& lhs,
         const bucket_iterator<Key, T, Container, isConst2, projectToConstKey>& rhs) noexcept
         -> bool
      {
         return lhs.current_node_index() != rhs.current_node_index();
      }

      //****************************************************//

      template <class key_, class any_, class container_, bool is_const_,
                bool project_to_const_key_>
      class small_dense_hash_map_iterator
      {
         friend small_dense_hash_map_iterator<key_, any_, container_, true, project_to_const_key_>;

      public:
         using container_type = container_;
         using sub_iterator_type =
            typename std::conditional_t<is_const_, typename container_type::const_iterator,
                                        typename container_type::iterator>;
         using sub_iterator_type_traits = std::iterator_traits<sub_iterator_type>;
         using projected_type =
            std::pair<typename std::conditional_t<project_to_const_key_, const key_, key_>, any_>;
         using iterator_category = typename sub_iterator_type_traits::iterator_category;
         using value_type = std::conditional_t<is_const_, const projected_type, projected_type>;
         using difference_type = typename sub_iterator_type_traits::difference_type;
         using reference = value_type&;
         using pointer = value_type*;

      public:
         constexpr small_dense_hash_map_iterator() noexcept = default;
         explicit constexpr small_dense_hash_map_iterator(sub_iterator_type it) noexcept :
            m_sub_iterator{it}
         {}

         // clang-format off
         template <bool dep_is_const_ = is_const_>
            requires (dep_is_const_ == true)
         constexpr small_dense_hash_map_iterator(
            // clang-format on
            const small_dense_hash_map_iterator<key_, any_, container_, false,
                                                project_to_const_key_>& other) noexcept :
            m_sub_iterator{other.m_sub_iterator}
         {}

         constexpr auto operator*() const noexcept -> reference
         {
            if constexpr (project_to_const_key_)
            {
               return m_sub_iterator->pair.const_key_pair();
            }
            else
            {
               return m_sub_iterator->pair.pair();
            }
         }

         constexpr auto operator->() const noexcept -> pointer
         {
            if constexpr (project_to_const_key_)
            {
               return &(m_sub_iterator->pair.const_key_pair());
            }
            else
            {
               return &(m_sub_iterator->pair.pair());
            }
         }

         constexpr auto operator++() noexcept -> small_dense_hash_map_iterator&
         {
            ++m_sub_iterator;
            return *this;
         }

         constexpr auto operator++(int) noexcept -> small_dense_hash_map_iterator
         {
            return {m_sub_iterator++};
         }

         constexpr auto operator--() noexcept -> small_dense_hash_map_iterator&
         {
            --m_sub_iterator;
            return *this;
         }
         constexpr auto operator--(int) noexcept -> small_dense_hash_map_iterator
         {
            return {m_sub_iterator--};
         }

         constexpr auto operator[](difference_type index) const noexcept -> reference
         {
            if constexpr (project_to_const_key_)
            {
               return m_sub_iterator[index]->pair.const_key_pair();
            }
            else
            {
               return m_sub_iterator[index]->pair.pair();
            }
         }

         constexpr auto operator+=(difference_type n) noexcept -> small_dense_hash_map_iterator&
         {
            m_sub_iterator += n;
            return *this;
         }
         constexpr auto operator+(difference_type n) const noexcept -> small_dense_hash_map_iterator
         {
            return {m_sub_iterator + n};
         }
         constexpr auto operator-=(difference_type n) noexcept -> small_dense_hash_map_iterator&
         {
            m_sub_iterator -= n;
            return *this;
         }
         constexpr auto operator-(difference_type n) const noexcept -> small_dense_hash_map_iterator
         {
            return {m_sub_iterator - n};
         }

         constexpr auto sub_iterator() const -> const sub_iterator_type& { return m_sub_iterator; }

      private:
         sub_iterator_type m_sub_iterator{sub_iterator_type{}};
      };

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey,
                bool isConst2>
      constexpr auto operator==(
         const small_dense_hash_map_iterator<Key, T, Container, isConst, projectToConstKey>& lhs,
         const small_dense_hash_map_iterator<Key, T, Container, isConst2, projectToConstKey>&
            rhs) noexcept -> bool
      {
         return lhs.sub_iterator() == rhs.sub_iterator();
      }

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey,
                bool isConst2>
      constexpr auto operator!=(
         const small_dense_hash_map_iterator<Key, T, Container, isConst, projectToConstKey>& lhs,
         const small_dense_hash_map_iterator<Key, T, Container, isConst2, projectToConstKey>&
            rhs) noexcept -> bool
      {
         return lhs.sub_iterator() != rhs.sub_iterator();
      }

      template <class key_, class any_, class container_, bool is_const_, bool is_const_2_,
                bool project_to_const_key_>
      constexpr auto
      operator<(const small_dense_hash_map_iterator<key_, any_, container_, is_const_,
                                                    project_to_const_key_>& lhs,
                const small_dense_hash_map_iterator<key_, any_, container_, is_const_2_,
                                                    project_to_const_key_>& rhs) noexcept -> bool
      {
         return lhs.sub_iterator() < rhs.sub_iterator();
      }

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey,
                bool isConst2>
      constexpr auto operator>(
         const small_dense_hash_map_iterator<Key, T, Container, isConst, projectToConstKey>& lhs,
         const small_dense_hash_map_iterator<Key, T, Container, isConst2, projectToConstKey>&
            rhs) noexcept -> bool
      {
         return lhs.sub_iterator() > rhs.sub_iterator();
      }

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey,
                bool isConst2>
      constexpr auto operator<=(
         const small_dense_hash_map_iterator<Key, T, Container, isConst, projectToConstKey>& lhs,
         const small_dense_hash_map_iterator<Key, T, Container, isConst2, projectToConstKey>&
            rhs) noexcept -> bool
      {
         return lhs.sub_iterator() <= rhs.sub_iterator();
      }

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey,
                bool isConst2>
      constexpr auto operator>=(
         const small_dense_hash_map_iterator<Key, T, Container, isConst, projectToConstKey>& lhs,
         const small_dense_hash_map_iterator<Key, T, Container, isConst2, projectToConstKey>&
            rhs) noexcept -> bool
      {
         return lhs.sub_iterator() >= rhs.sub_iterator();
      }

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey,
                bool isConst2>
      constexpr auto operator-(
         const small_dense_hash_map_iterator<Key, T, Container, isConst, projectToConstKey>& lhs,
         const small_dense_hash_map_iterator<Key, T, Container, isConst2, projectToConstKey>&
            rhs) noexcept ->
         typename small_dense_hash_map_iterator<Key, T, Container, isConst,
                                                projectToConstKey>::difference_type
      {
         return lhs.sub_iterator() - rhs.sub_iterator();
      }

      template <class Key, class T, class Container, bool isConst, bool projectToConstKey>
      constexpr auto
      operator+(typename small_dense_hash_map_iterator<Key, T, Container, isConst,
                                                       projectToConstKey>::difference_type n,
                const small_dense_hash_map_iterator<Key, T, Container, isConst, projectToConstKey>&
                   it) noexcept
         -> small_dense_hash_map_iterator<Key, T, Container, isConst, projectToConstKey>
      {
         return {n + it.sub_iterator()};
      }

   } // namespace detail

   template <class key_, class any_, size_t buffer_size_, class hash_ = std::hash<key_>,
             class pred_ = std::equal_to<key_>,
             allocator allocator_ = std::allocator<std::pair<const key_, any_>>>
   class small_dense_hash_map
   {
      using node_type = detail::node<key_, any_>;
      using node_index_type = typename node_type::index_t;
      using nodes_container_type = small_dynamic_array<node_type, buffer_size_>;
      using nodes_size_type = typename nodes_container_type::size_type;
      using buckets_container_type =
         small_dynamic_array<nodes_size_type, detail::max_bucket_container_size(buffer_size_)>;
      using deduced_key_equal = typename detail::key_equal<hash_, pred_, key_>::type;

      static inline constexpr bool is_nothrow_move_constructible =
         std::allocator_traits<allocator_>::is_always_equal::value &&
         std::is_nothrow_move_constructible_v<hash_> &&
         std::is_nothrow_move_constructible_v<deduced_key_equal> &&
         std::is_nothrow_move_constructible_v<nodes_container_type> &&
         std::is_nothrow_move_constructible_v<buckets_container_type>;
      static inline constexpr bool is_nothrow_move_assignable =
         std::allocator_traits<allocator_>::is_always_equal::value &&
         std::is_nothrow_move_assignable_v<hash_> &&
         std::is_nothrow_move_assignable_v<deduced_key_equal> &&
         std::is_nothrow_move_assignable_v<nodes_container_type> &&
         std::is_nothrow_move_assignable_v<buckets_container_type>;
      static inline constexpr bool is_nothrow_swappable =
         std::is_nothrow_swappable_v<buckets_container_type> &&
         std::is_nothrow_swappable_v<nodes_container_type> &&
         std::allocator_traits<allocator_>::is_always_equal::value &&
         std::is_nothrow_swappable_v<hash_> && std::is_nothrow_swappable_v<deduced_key_equal>;
      static inline constexpr bool is_nothrow_default_constructible =
         std::is_nothrow_default_constructible_v<buckets_container_type> &&
         std::is_nothrow_default_constructible_v<nodes_container_type> &&
         std::is_nothrow_default_constructible_v<hash_> &&
         std::is_nothrow_default_constructible_v<deduced_key_equal>;

   public:
      using key_type = key_;
      using mapped_type = any_;
      using value_type = std::pair<const key_type, mapped_type>;
      using size_type = nodes_size_type;
      using difference_type = typename nodes_container_type::difference_type;
      using hasher = hash_;
      using key_equal = typename detail::key_equal<hasher, pred_, key_type>::type;
      using allocator_type = allocator_;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
      using iterator = detail::small_dense_hash_map_iterator<key_type, mapped_type,
                                                             nodes_container_type, false, true>;
      using const_iterator =
         detail::small_dense_hash_map_iterator<key_type, mapped_type, nodes_container_type, true,
                                               true>;
      using local_iterator =
         detail::bucket_iterator<key_type, mapped_type, nodes_container_type, false, true>;
      using const_local_iterator =
         detail::bucket_iterator<key_type, mapped_type, nodes_container_type, true, true>;

   public:
      constexpr small_dense_hash_map() noexcept(is_nothrow_default_constructible) :
         small_dense_hash_map{buffer_size_}
      {}

      constexpr explicit small_dense_hash_map(size_type bucket_count, const hasher& hash = hasher(),
                                              const key_equal& equal = key_equal(),
                                              const allocator_type& alloc = allocator_type()) :
         m_hash{hash},
         m_key_equal{equal}, m_nodes{alloc}, m_buckets{alloc}
      {
         rehash(bucket_count);
      }

      constexpr small_dense_hash_map(size_type bucket_count, const allocator_type& alloc) :
         small_dense_hash_map{bucket_count, hasher(), key_equal(), alloc}
      {}

      constexpr small_dense_hash_map(size_type bucket_count, const hasher& hash,
                                     const allocator_type& alloc) :
         small_dense_hash_map{bucket_count, hash, key_equal(), alloc}
      {}

      constexpr explicit small_dense_hash_map(const allocator_type& alloc) :
         small_dense_hash_map{buffer_size_, hasher(), key_equal(), alloc}
      {}

      template <class InputIt>
      constexpr small_dense_hash_map(InputIt first, InputIt last,
                                     size_type bucket_count = buffer_size_,
                                     const hasher& hash = hasher(),
                                     const key_equal& equal = key_equal(),
                                     const allocator_type& alloc = allocator_type()) :
         small_dense_hash_map{bucket_count, hash, equal, alloc}
      {
         insert(first, last);
      }

      template <class InputIt>
      constexpr small_dense_hash_map(InputIt first, InputIt last, size_type bucket_count,
                                     const allocator_type& alloc) :
         small_dense_hash_map{first, last, bucket_count, hasher(), key_equal(), alloc}
      {}

      template <class InputIt>
      constexpr small_dense_hash_map(InputIt first, InputIt last, size_type bucket_count,
                                     const hasher& hash, const allocator_type& alloc) :
         small_dense_hash_map{first, last, bucket_count, hash, key_equal(), alloc}
      {}

      constexpr small_dense_hash_map(const small_dense_hash_map& other) :
         small_dense_hash_map{
            other,
            std::allocator_traits<allocator_type>::select_on_container_copy_construction(
               other.get_allocator())}
      {}

      constexpr small_dense_hash_map(const small_dense_hash_map& other,
                                     const allocator_type& alloc) :
         m_hash{other.m_hash},
         m_key_equal{other.m_key_equal}, m_nodes{other.m_nodes, alloc}, m_buckets{other.m_buckets,
                                                                                  alloc}
      {}

      constexpr small_dense_hash_map(small_dense_hash_map&& other) noexcept(
         is_nothrow_move_constructible) = default;

      constexpr small_dense_hash_map(small_dense_hash_map&& other, const allocator_type& alloc) :
         m_hash{std::move(other.m_hash)}, m_key_equal{std::move(other.m_key_equal)},
         m_nodes{std::move(other.m_nodes), alloc}, m_buckets{std::move(other.m_buckets), alloc}
      {}

      constexpr small_dense_hash_map(std::initializer_list<value_type> init,
                                     size_type bucket_count = buffer_size_,
                                     const hasher& hash = hasher(),
                                     const key_equal& equal = key_equal(),
                                     const allocator_type& alloc = allocator_type()) :
         small_dense_hash_map{std::begin(init), std::end(init), bucket_count, hash, equal, alloc}
      {}

      constexpr small_dense_hash_map(std::initializer_list<value_type> init, size_type bucket_count,
                                     const allocator_type& alloc) :
         small_dense_hash_map{init, bucket_count, hasher(), key_equal(), alloc}
      {}

      constexpr small_dense_hash_map(std::initializer_list<value_type> init, size_type bucket_count,
                                     const hasher& hash, const allocator_type& alloc) :
         small_dense_hash_map{init, bucket_count, hash, key_equal(), alloc}
      {}

      // 2 missing constructors from https://cplusplus.github.io/LWG/issue2713
      template <std::input_iterator it_>
      constexpr small_dense_hash_map(it_ first, it_ last, const allocator_type& alloc) :
         small_dense_hash_map{first, last, buffer_size_, hasher(), key_equal(), alloc}
      {}

      constexpr small_dense_hash_map(std::initializer_list<value_type> init,
                                     const allocator_type& alloc) :
         small_dense_hash_map{init, buffer_size_, hasher(), key_equal(), alloc}
      {}

      constexpr ~small_dense_hash_map() = default;

      constexpr auto operator=(const small_dense_hash_map& other)
         -> small_dense_hash_map& = default;
      constexpr auto operator=(small_dense_hash_map&& other) noexcept(is_nothrow_move_assignable)
         -> small_dense_hash_map& = default;

      constexpr auto operator=(std::initializer_list<value_type> ilist) -> small_dense_hash_map&
      {
         clear();
         insert(std::begin(ilist), std::end(ilist));
         return *this;
      }

      constexpr auto get_allocator() const -> allocator_type { return m_buckets.get_allocator(); }

      constexpr auto begin() noexcept -> iterator { return iterator{std::begin(m_nodes)}; }
      constexpr auto begin() const noexcept -> const_iterator
      {
         return const_iterator{std::begin(m_nodes)};
      }
      constexpr auto cbegin() const noexcept -> const_iterator
      {
         return const_iterator{std::begin(m_nodes)};
      }

      constexpr auto end() noexcept -> iterator { return iterator{std::end(m_nodes)}; }
      constexpr auto end() const noexcept -> const_iterator
      {
         return const_iterator{std::end(m_nodes)};
      }
      constexpr auto cend() const noexcept -> const_iterator
      {
         return const_iterator{std::end(m_nodes)};
      }

      [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_nodes.empty(); }
      constexpr auto size() const noexcept -> size_type { return std::size(m_nodes); }
      constexpr auto max_size() const noexcept -> size_type { return m_nodes.max_size(); }
      constexpr void clear() noexcept
      {
         m_nodes.clear();
         m_buckets.clear();

         rehash(0u);
      }

      constexpr auto insert(const value_type& value) -> std::pair<iterator, bool>
      {
         return emplace(value);
      }

      constexpr auto insert(value_type&& value) -> std::pair<iterator, bool>
      {
         return emplace(std::move(value));
      }

      constexpr auto insert(auto&& value)
         -> std::pair<iterator, bool> requires std::constructible_from<value_type, decltype(value)>
      {
         return emplace(std::forward<decltype(value)>(value));
      }

      constexpr auto insert(const_iterator /*hint*/, const value_type& value) -> iterator
      {
         return insert(value).first;
      }

      constexpr auto insert(const_iterator /*hint*/, value_type&& value) -> iterator
      {
         return insert(std::move(value)).first;
      }

      constexpr auto insert(const_iterator /*hint*/, auto&& value) -> iterator
         requires std::constructible_from<value_type, decltype(value)>
      {
         return insert(std::move(value)).first;
      }

      template <class InputIt>
      constexpr void insert(InputIt first, InputIt last)
      {
         for (; first != last; ++first)
         {
            insert(*first);
         }
      }

      constexpr void insert(std::initializer_list<value_type> ilist)
      {
         insert(std::begin(ilist), std::begin(ilist));
      }

      constexpr auto insert_or_assign(const key_type& k, auto&& obj) -> std::pair<iterator, bool>
      {
         auto result = try_emplace(k, std::forward<decltype(obj)>(obj));

         if (!result.second)
         {
            result.first->second = std::forward<decltype(obj)>(obj);
         }

         return result;
      }

      constexpr auto insert_or_assign(key_type&& k, auto&& obj) -> std::pair<iterator, bool>
      {
         auto result = try_emplace(std::move(k), std::forward<decltype(obj)>(obj));

         if (!result.second)
         {
            result.first->second = std::forward<decltype(obj)>(obj);
         }

         return result;
      }

      constexpr auto insert_or_assign(const_iterator /*hint*/, const key_type& k, auto&& obj)
         -> iterator
      {
         return insert_or_assign(k, std::forward<decltype(obj)>(obj)).first;
      }

      constexpr auto insert_or_assign(const_iterator /*hint*/, key_type&& k, auto&& obj) -> iterator
      {
         return insert_or_assign(std::move(k), std::forward<decltype(obj)>(obj)).first;
      }

      constexpr auto emplace(auto&&... args) -> std::pair<iterator, bool>
      {
         return dispatch_emplace(std::forward<decltype(args)>(args)...);
      }

      constexpr auto emplace_hint(const_iterator /*hint*/, auto&&... args) -> iterator
      {
         return emplace(std::forward<decltype(args)>(args)...).first;
      }

      constexpr auto try_emplace(const key_type& key, auto&&... args) -> std::pair<iterator, bool>
      {
         return do_emplace(key, std::piecewise_construct, std::forward_as_tuple(key),
                           std::forward_as_tuple(std::forward<decltype(args)>(args)...));
      }

      constexpr auto try_emplace(key_type&& key, auto&&... args) -> std::pair<iterator, bool>
      {
         return do_emplace(key, std::piecewise_construct, std::forward_as_tuple(std::move(key)),
                           std::forward_as_tuple(std::forward<decltype(args)>(args)...));
      }

      constexpr auto try_emplace(const_iterator /*hint*/, const key_type& key, auto&&... args)
         -> iterator
      {
         return try_emplace(key, std::forward<decltype(args)>(args)...).iterator;
      }

      constexpr auto try_emplace(const_iterator /*hint*/, key_type&& key, auto&&... args)
         -> iterator
      {
         return try_emplace(std::move(key), std::forward<decltype(args)>(args)...).iterator;
      }

      constexpr auto erase(const_iterator pos) -> iterator
      {
         const auto position = std::distance(cbegin(), pos);
         const auto it = std::next(begin(), position);
         const auto previous_next = find_previous_next_using_position(pos->first, position);
         return do_erase(previous_next, it.sub_iterator()).first;
      }

      constexpr auto erase(const_iterator first, const_iterator last) -> iterator
      {
         bool stop = first == last;
         while (!stop)
         {
            --last;
            stop = first == last; // if first == last, erase would invalidate both!
            last = erase(last);
         }

         const auto position = std::distance(cbegin(), last);
         return std::next(begin(), position);
      }

      constexpr auto erase(const key_type& key) -> size_type
      {
         // We have to find out the node we look for and the pointer to it.
         const auto bindex = bucket_index(key);

         std::size_t* previous_next = &m_buckets[bindex];

         for (;;)
         {
            if (*previous_next == std::numeric_limits<node_index_type>::max())
            {
               return 0;
            }

            auto& node = m_nodes[*previous_next];

            if (m_key_equal(node.pair.pair().first, key))
            {
               break;
            }

            previous_next = &node.next;
         }

         do_erase(previous_next, std::next(std::begin(m_nodes), *previous_next));

         return 1;
      }

      /*
      constexpr void swap(small_dense_hash_map& other) noexcept(is_nothrow_swappable)
      {
         using std::swap;
         swap(m_buckets, other.m_buckets);
         swap(m_nodes, other.m_nodes);
         swap(m_max_load_factor, other.m_max_load_factor);
         swap(m_hash, other.m_hash);
         swap(m_key_equal, other.m_key_equal);
      }
      */

      constexpr auto at(const key_type& key) -> mapped_type&
      {
         const auto it = find(key);

         if (it == end())
         {
            handle_out_of_range_error("The specified key does not exists in this map.");
         }

         return it->second;
      }

      constexpr auto at(const key_type& key) const -> const mapped_type&
      {
         const auto it = find(key);

         if (it == end())
         {
            handle_out_of_range_error("The specified key does not exists in this map.");
         }

         return it->second;
      }

      constexpr auto operator[](const key_type& key) -> mapped_type&
      {
         return this->try_emplace(key).first->second;
      }

      constexpr auto operator[](key_type&& key) -> mapped_type&
      {
         return this->try_emplace(std::move(key)).first->second;
      }

      constexpr auto count(const key_type& key) const -> size_type
      {
         return find(key) == end() ? 0u : 1u;
      }

      template <
         class in_key_,
         class Useless = std::enable_if_t<detail::is_transparent_key_equal_v<hash_>, in_key_>>
      constexpr auto count(const in_key_& key) const -> size_type
      {
         return find(key) == end() ? 0u : 1u;
      }

      constexpr auto find(const key_type& key) -> iterator
      {
         return bucket_iterator_to_iterator(find_in_bucket(key, bucket_index(key)), m_nodes);
      }

      constexpr auto find(const key_type& key) const -> const_iterator
      {
         return bucket_iterator_to_iterator(find_in_bucket(key, bucket_index(key)), m_nodes);
      }

      template <
         class in_key_,
         class Useless = std::enable_if_t<detail::is_transparent_key_equal_v<hash_>, in_key_>>
      constexpr auto find(const in_key_& key) -> iterator
      {
         return bucket_iterator_to_iterator(find_in_bucket(key, bucket_index(key)), m_nodes);
      }

      template <
         class in_key_,
         class Useless = std::enable_if_t<detail::is_transparent_key_equal_v<hash_>, in_key_>>
      constexpr auto find(const in_key_& key) const -> const_iterator
      {
         return bucket_iterator_to_iterator(find_in_bucket(key, bucket_index(key)), m_nodes);
      }

      constexpr auto contains(const key_type& key) const -> bool { return find(key) != end(); }

      template <
         class in_key_,
         class Useless = std::enable_if_t<detail::is_transparent_key_equal_v<hash_>, in_key_>>
      constexpr auto contains(const in_key_& key) const -> bool
      {
         return find(key) != end();
      }

      constexpr auto equal_range(const key_type& key) -> std::pair<iterator, iterator>
      {
         const auto it = find(key);

         if (it == end())
         {
            return {it, it};
         }

         return {it, std::next(it)};
      }
      constexpr auto equal_range(const key_type& key) const
         -> std::pair<const_iterator, const_iterator>
      {
         const auto it = find(key);

         if (it == end())
         {
            return {it, it};
         }

         return {it, std::next(it)};
      }
      template <
         class in_key_,
         class Useless = std::enable_if_t<detail::is_transparent_key_equal_v<hash_>, in_key_>>
      constexpr auto equal_range(const in_key_& key) -> std::pair<iterator, iterator>
      {
         const auto it = find(key);

         if (it == end())
         {
            return {it, it};
         }

         return {it, std::next(it)};
      }
      template <
         class in_key_,
         class Useless = std::enable_if_t<detail::is_transparent_key_equal_v<hash_>, in_key_>>
      constexpr auto equal_range(const in_key_& key) const
         -> std::pair<const_iterator, const_iterator>
      {
         const auto it = find(key);

         if (it == end())
         {
            return {it, it};
         }

         return {it, std::next(it)};
      }

      constexpr auto begin(size_type n) -> local_iterator
      {
         return local_iterator{m_buckets[n], m_nodes};
      }
      constexpr auto begin(size_type n) const -> const_local_iterator
      {
         return const_local_iterator{m_buckets[n], m_nodes};
      }
      constexpr auto cbegin(size_type n) const -> const_local_iterator
      {
         return const_local_iterator{m_buckets[n], m_nodes};
      }

      constexpr auto end(size_type /*n*/) -> local_iterator { return local_iterator{m_nodes}; }
      constexpr auto end(size_type /*n*/) const -> const_local_iterator
      {
         return const_local_iterator{m_nodes};
      }
      constexpr auto cend(size_type /*n*/) const -> const_local_iterator
      {
         return const_local_iterator{m_nodes};
      }

      constexpr auto bucket_count() const -> size_type { return std::size(m_buckets); }

      constexpr auto max_bucket_count() const -> size_type { return m_buckets.max_size(); }

      constexpr auto bucket_size(size_type n) const -> size_type
      {
         return static_cast<size_t>(std::distance(begin(n), end(n)));
      }

      constexpr auto bucket(const key_type& key) const -> size_type { return bucket_index(key); }

      [[nodiscard]] constexpr auto load_factor() const -> float
      {
         return size() / static_cast<float>(bucket_count());
      }

      [[nodiscard]] constexpr auto max_load_factor() const -> float { return m_max_load_factor; }

      constexpr void max_load_factor(float ml)
      {
         assert(ml > 0.0f && "The max load factor must be greater than 0.0f.");
         m_max_load_factor = ml;

         rehash(detail::min_bucket_container_size);
      }

      constexpr void rehash(size_type count)
      {
         count = std::max(detail::min_bucket_container_size, count);
         count =
            std::max(count, static_cast<size_type>(static_cast<float>(size()) / max_load_factor()));

         count = compute_new_capacity(count);

         assert(count > 0 && "The computed rehash size must be greater than 0.");

         if (count == std::size(m_buckets))
         {
            return;
         }

         m_buckets.resize(count);

         std::fill(std::begin(m_buckets), std::end(m_buckets),
                   std::numeric_limits<nodes_size_type>::max());

         node_index_type index{0u};

         for (auto& entry : m_nodes)
         {
            entry.next = std::numeric_limits<nodes_size_type>::max();
            reinsert_entry(entry, index);
            index++;
         }
      }

      constexpr void reserve(std::size_t count)
      {
         rehash(std::ceil(count / max_load_factor()));
         m_nodes.reserve(count);
      }

      constexpr auto hash_function() const -> hasher { return m_hash; }
      constexpr auto key_eq() const -> key_equal { return m_key_equal; }

   private:
      static constexpr auto compute_new_capacity(std::size_t min_capacity) -> std::size_t
      {
         constexpr auto max_capacity = std::size_t{1}
            << (std::numeric_limits<std::size_t>::digits - 1);

         if (min_capacity > max_capacity)
         {
            return max_capacity;
         }

         --min_capacity;

         for (auto i = 1U; i < std::numeric_limits<std::size_t>::digits; i *= 2)
         {
            min_capacity |= min_capacity >> i;
         }

         return ++min_capacity;
      }

      constexpr auto bucket_index(const auto& key) const -> size_type
      {
         return m_hash(key) & (std::size(m_buckets) - 1);
      }

      constexpr auto find_in_bucket(const auto& key, std::size_t bucket_index) -> local_iterator
      {
         auto beg = begin(bucket_index);
         return std::find_if(beg, end(0u), [&key, this](auto& p) {
            return m_key_equal(p.first, key);
         });
      }

      constexpr auto find_in_bucket(const auto& key, std::size_t bucket_index) const
         -> const_local_iterator
      {
         auto beg = begin(bucket_index);
         return std::find_if(beg, end(0u), [&key, this](auto& p) {
            return m_key_equal(p.first, key);
         });
      }

      constexpr auto do_erase(std::size_t* previous_next,
                              typename nodes_container_type::iterator sub_it)
         -> std::pair<iterator, bool>
      {
         // Skip the node by pointing the previous "next" to the one sub_it currently point to.
         *previous_next = sub_it->next;

         auto last = std::prev(std::end(m_nodes));

         // No need to do anything if the node was at the end of the vector.
         if (sub_it == last)
         {
            m_nodes.pop_back();
            return {end(), true};
         }

         // Swap last node and the one we want to delete.
         using std::swap;
         swap(*sub_it, *last);

         // Now sub_it points to the one we swapped with. We have to readjust sub_it.
         previous_next =
            find_previous_next_using_position(sub_it->pair.pair().first, std::size(m_nodes) - 1);
         *previous_next = std::distance(std::begin(m_nodes), sub_it);

         // Delete the last node forever and ever.
         m_nodes.pop_back();

         return {iterator{sub_it}, true};
      }

      constexpr auto find_previous_next_using_position(const key_type& key, std::size_t position)
         -> std::size_t*
      {
         const std::size_t bindex = bucket_index(key);

         auto previous_next = &m_buckets[bindex];
         while (*previous_next != position)
         {
            previous_next = &m_nodes[*previous_next].next;
         }

         return previous_next;
      }

      constexpr void reinsert_entry(node_type& entry, node_index_type index)
      {
         const auto bindex = bucket_index(entry.pair.const_key_pair().first);
         auto old_index = std::exchange(m_buckets[bindex], index);
         entry.next = old_index;
      }

      constexpr void check_for_rehash()
      {
         if (size() + 1 > bucket_count() * max_load_factor())
         {
            rehash(bucket_count() * 2);
         }
      }

      constexpr auto dispatch_emplace() -> std::pair<iterator, bool>
      {
         return do_emplace(key_type{});
      }

      constexpr auto dispatch_emplace(auto&& key, auto&& t) -> std::pair<iterator, bool>
      {
         if constexpr (std::is_same_v<std::decay_t<decltype(key)>, key_type>)
         {
            return do_emplace(key, std::forward<decltype(key)>(key), std::forward<decltype(t)>(t));
         }
         else
         {
            key_type new_key{std::forward<decltype(key)>(key)};
            // TODO: double check that I am allowed to optimized by sending new_key here and not key
            // https://eel.is/c++draft/unord.req#lib:emplace,unordered_associative_containers
            return do_emplace(new_key, std::move(new_key), std::forward<decltype(t)>(t));
         }
      }

      constexpr auto dispatch_emplace(auto&& p) -> std::pair<iterator, bool>
      {
         if constexpr (std::is_same_v<std::decay_t<decltype(p.first)>, key_type>)
         {
            return do_emplace(p.first, std::forward<decltype(p)>(p));
         }
         else
         {
            key_type new_key{std::forward<decltype(p)>(p).first};
            return do_emplace(new_key, std::move(new_key), std::forward<decltype(p)>(p).second);
         }
      }

      template <class... args_one_, class... args_two_>
      constexpr auto dispatch_emplace(std::piecewise_construct_t,
                                      std::tuple<args_one_...> first_args,
                                      std::tuple<args_two_...> second_args)
         -> std::pair<iterator, bool>
      {
         std::pair<key_type, mapped_type> p{std::piecewise_construct, std::move(first_args),
                                            std::move(second_args)};
         return dispatch_emplace(std::move(p));
      }

      constexpr auto do_emplace(const key_type& key, auto&&... args) -> std::pair<iterator, bool>
      {
         check_for_rehash();

         const auto bindex = bucket_index(key);
         auto local_it = find_in_bucket(key, bindex);

         if (local_it != end(0u))
         {
            return std::pair{bucket_iterator_to_iterator(local_it, m_nodes), false};
         }

         m_nodes.emplace_back(m_buckets[bindex], std::forward<decltype(args)>(args)...);
         m_buckets[bindex] = std::size(m_nodes) - 1;

         return std::pair{std::prev(end()), true};
      }

      template <class container_, bool is_const_, bool project_to_const_key_>
      constexpr auto bucket_iterator_to_iterator(
         const detail::bucket_iterator<key_type, mapped_type, container_, is_const_,
                                       project_to_const_key_>& bucket_it,
         auto& nodes) -> iterator
      {
         if (bucket_it.current_node_index() == std::numeric_limits<node_index_type>::max())
         {
            return iterator{std::end(nodes)};
         }
         else
         {
            return iterator{std::next(std::begin(nodes), bucket_it.current_node_index())};
         }
      }

      template <class container_, bool is_const_, bool project_to_const_key_>
      constexpr auto bucket_iterator_to_iterator(
         const detail::bucket_iterator<key_type, mapped_type, container_, is_const_,
                                       project_to_const_key_>& bucket_it,
         auto& nodes) const -> const_iterator
      {
         if (bucket_it.current_node_index() == std::numeric_limits<node_index_type>::max())
         {
            return const_iterator{std::end(nodes)};
         }
         else
         {
            return const_iterator{std::next(std::begin(nodes), bucket_it.current_node_index())};
         }
      }

   private:
      hasher m_hash;
      key_equal m_key_equal;

      nodes_container_type m_nodes;
      buckets_container_type m_buckets;

      float m_max_load_factor = 0.875f; // NOLINT
   };

   template <class key_, class any_, size_t buff_sz_lhs_, size_t buff_sz_rhs_, class hash_,
             class key_eq_, allocator allocator_>
   constexpr auto
   operator==(const small_dense_hash_map<key_, any_, buff_sz_lhs_, hash_, key_eq_, allocator_>& lhs,
              const small_dense_hash_map<key_, any_, buff_sz_rhs_, hash_, key_eq_, allocator_>& rhs)
      -> bool
   {
      if (std::size(lhs) != std::size(rhs))
      {
         return false;
      }

      for (const auto& item : lhs)
      {
         const auto it = rhs.find(item.first);

         if (it == std::end(rhs) || it->second != item.second)
         {
            return false;
         }
      }

      return true;
   }

   template <class key_, class any_, class hash_ = std::hash<key_>,
             class pred_ = std::equal_to<key_>,
             allocator allocator_ = std::allocator<std::pair<const key_, any_>>>
   using dense_hash_map = small_dense_hash_map<key_, any_, 0, hash_, pred_, allocator_>;

   namespace pmr
   {
      template <class key_, class any_, std::size_t buff_sz_, class hash_ = std::hash<key_>,
                class pred_ = std::equal_to<key_>>
      using small_dense_hash_map =
         ::util::small_dense_hash_map<key_, any_, buff_sz_, hash_, pred_,
                                      std::pmr::polymorphic_allocator<std::pair<key_, any_>>>;

      template <class key_, class any_, class hash_ = std::hash<key_>,
                class pred_ = std::equal_to<key_>>
      using dense_hash_map =
         ::util::small_dense_hash_map<key_, any_, 0, hash_, pred_,
                                      std::pmr::polymorphic_allocator<std::pair<key_, any_>>>;
   } // namespace pmr
} // namespace util
