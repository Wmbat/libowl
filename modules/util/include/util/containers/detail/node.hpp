
#pragma once

#include <util/containers/detail/disabling_structs.hpp>

#include <cstddef>
#include <limits>
#include <utility>

namespace util::detail
{
   template <class Key, class T>
   union union_key_value_pair
   {
      using pair_t = std::pair<Key, T>;
      using const_key_pair_t = std::pair<const Key, T>;

      static_assert(offsetof(pair_t, first) == offsetof(const_key_pair_t, first) &&
         offsetof(pair_t, second) == offsetof(const_key_pair_t, second));

      union_key_value_pair(auto&&... args) : pair_(std::forward<decltype(args)>(args)...) {}
      union_key_value_pair(const union_key_value_pair& other) {}
      union_key_value_pair(union_key_value_pair&& other) noexcept {}

      auto operator=(const union_key_value_pair& other) -> union_key_value_pair&
      {
         if (this != &other)
         {
            pair_ = other.pair_;
         }

         return *this;
      }
      auto operator=(union_key_value_pair&& other) noexcept -> union_key_value_pair&
      {
         pair_ = std::move(other).pair_;

         return *this;
      }

      ~union_key_value_pair() { pair_.~pair_t(); }

      auto pair() -> pair_t& { return pair_; }
      auto pair() const -> const pair_t& { return pair_; }
      auto const_key_pair() -> const_key_pair_t& { return const_key_pair_; }
      auto const_key_pair() const -> const const_key_pair_t& { return const_key_pair_; }

   private:
      pair_t pair_; //!< active member
      const_key_pair_t const_key_pair_;
   };

   template <class key_, class any_>
   struct safe_key_value_pair
   {
      using const_key_pair_t = std::pair<const key_, any_>;

      safe_key_value_pair(auto&&... args) : const_key_pair_(std::forward<decltype(args)>(args)...)
      {}

      // Accessors to the two members.
      auto pair() -> const_key_pair_t& { return const_key_pair_; }
      auto pair() const -> const const_key_pair_t& { return const_key_pair_; }
      auto const_key_pair() -> const_key_pair_t& { return const_key_pair_; }
      auto const_key_pair() const -> const const_key_pair_t& { return const_key_pair_; }

   private:
      const_key_pair_t const_key_pair_;
   };

   // clang-format off
      template <class key_, class any_>
      inline constexpr bool are_pairs_standard_layout_v =
         std::is_standard_layout_v<std::pair<const key_, any_>> &&
         std::is_standard_layout_v<std::pair<key_, any_>>;
   // clang-format on

   // clang-format off
      template <class key_, class any_>
      using key_value_pair_t = std::conditional_t<
         are_pairs_standard_layout_v<key_, any_>,
         union_key_value_pair<key_, any_>, 
         safe_key_value_pair<key_, any_>
      >;
   // clang-format on

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

} // namespace util::detail
