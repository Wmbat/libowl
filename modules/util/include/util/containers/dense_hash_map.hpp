/**
 * @file dense_hash_map.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date 17th of July, 2020.
 * @copyright MIT License.
 *
 * Follows closely this implementation of a dense hash map: https://github.com/Jiwan/dense_hash_map
 */

#include <util/containers/detail/disabling_structs.hpp>
#include <util/containers/detail/growth_policy.hpp>
#include <util/containers/detail/node.hpp>
#include <util/containers/dynamic_array.hpp>
#include <util/type_traits.hpp>

#include <functional>
#include <ranges>

namespace util
{
   namespace detail
   {
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

         static_assert(is_transparent_v<type>,
            "The associated transparent key equal is missing a is_transparent tag type.");
         static_assert(std::is_same_v<Pred, std::equal_to<Key>> || std::is_same_v<Pred, type>,
            "The associated transparent key equal must be the transparent_key_equal tag or "
            "std::equal_to<Key>");
      };

      static inline constexpr size_t min_bucket_container_size = 8u;

      consteval auto max_bucket_container_size(size_t size) noexcept -> size_t
      {
         return size >= min_bucket_container_size ? size : min_bucket_container_size;
      }

   } // namespace detail

   template <class key_, class any_, size_t buffer_size_, class hash_ = std::hash<key_>,
      class pred_ = std::equal_to<key_>>
   class small_dense_hash_map
   {
      using node_type = detail::node<key_, any_>;
      using node_index_type = typename node_type::index_t;
      using nodes_container_type = small_dynamic_array<node_type, buffer_size_>;
      using nodes_size_type = typename nodes_container_type::size_type;
      using buckets_container_type =
         small_dynamic_array<nodes_size_type, detail::max_bucket_container_size(buffer_size_)>;

   public:
      using key_type = key_;
      using mapped_type = any_;
      using value_type = std::pair<const key_type, mapped_type>;
      using size_type = nodes_size_type;
      using difference_type = typename nodes_container_type::difference_type;
      using hasher = hash_;
      using key_equal = typename detail::key_equal<hasher, pred_, key_type>::type;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using iterator = random_access_iterator<value_type>;
      using const_iterator = random_access_iterator<const value_type>;

      constexpr auto begin() noexcept -> iterator { return projected_range().begin(); }
      constexpr auto begin() const noexcept -> const_iterator { return projected_range().begin(); }
      constexpr auto cbegin() const noexcept -> const_iterator { return projected_range().begin(); }

      constexpr auto end() noexcept -> iterator { return projected_range().end(); }
      constexpr auto end() const noexcept -> const_iterator { return projected_range().end(); }
      constexpr auto cend() const noexcept -> const_iterator { return projected_range().end(); }

      [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_nodes.empty(); }
      constexpr auto size() const noexcept -> size_type { return m_nodes.size(); }
      constexpr auto max_size() const noexcept -> size_type { return m_nodes.max_size(); }
      constexpr void clear() noexcept
      {
         m_nodes.clear();
         m_buckets.clear();

         rehash(0u);
      }

      constexpr auto bucket_count() const -> size_type { return m_buckets.size(); }

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
         count = std::max(count, static_cast<size_type>(size() / max_load_factor()));

         count = compute_closest_capacity(count);

         assert(count > 0 && "The computed rehash size must be greater than 0.");

         if (count == m_buckets.size())
         {
            return;
         }

         m_buckets.resize(count);

         std::fill(m_buckets.begin(), m_buckets.end(), std::numeric_limits<nodes_size_type>::max());

         node_index_type index{0u};

         for (auto& entry : m_nodes)
         {
            entry.next = std::numeric_limits<nodes_size_type>::max();
            reinsert_entry(entry, index);
            index++;
         }
      }

      constexpr auto hash_function() const -> hasher { return m_hash; }
      constexpr auto key_eq() const -> key_equal { return m_key_equal; }

   private:
      constexpr auto projected_range()
      {
         return m_nodes | std::views::transform([](auto& node) {
            return node.pair.pair();
         });
      }

      constexpr auto bucket_index(const auto& key) const -> size_type
      {
         return compute_index(hash_(key), m_buckets.size());
      }

   private:
      hasher m_hash;
      key_equal m_key_equal;

      nodes_container_type m_nodes;
      buckets_container_type m_buckets;

      float m_max_load_factor = 0.875f;
   };

   template <class key_, class any_, class hash_ = std::hash<key_>,
      class pred_ = std::equal_to<key_>>
   using dense_hash_map = small_dense_hash_map<key_, any_, 0, hash_, pred_>;
} // namespace util
