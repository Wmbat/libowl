#pragma once

#include <epona_core/detail/concepts.hpp>

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>
#include <variant>

namespace core
{
   namespace detail
   {
      template <class any_>
      struct maybe_storage
      {
         using value_type = any_;

         maybe_storage() = default;
         maybe_storage(const value_type& value) : is_engaged{true}
         {
            if (is_engaged)
            {
               new (data.data()) value_type{value};
            }
         }
         maybe_storage(value_type&& value) : is_engaged{true}
         {
            if (is_engaged)
            {
               new (data.data()) value_type{std::move(value)};
            }
         }
         maybe_storage(const maybe_storage& rhs) : is_engaged{rhs.is_engaged}
         {
            if (is_engaged)
            {
               new (data.data()) value_type{*static_cast<value_type*>(data.data())};
            }
         }
         maybe_storage(maybe_storage&& rhs) noexcept : is_engaged{rhs.is_engaged}
         {
            if (is_engaged)
            {
               new (data.data()) value_type{std::move(*static_cast<value_type*>(data.data()))};
            }
         }
         ~maybe_storage()
         {
            if (is_engaged)
            {
               reinterpret_cast<value_type*>(data.data())->~value_type(); // NOLINT
            }
         }

         auto operator=(const maybe_storage& rhs) -> maybe_storage&
         {
            if (this != &rhs)
            {
               if (is_engaged)
               {
                  reinterpret_cast<value_type*>(data.data())->~value_type(); // NOLINT
               }

               is_engaged = rhs.is_engaged;

               if (is_engaged)
               {
                  new (data.data())
                     value_type{*reinterpret_cast<value_type*>(rhs.data.data())}; // NOLINT
               }
            }

            return *this;
         }
         auto operator=(maybe_storage&& rhs) noexcept -> maybe_storage&
         {
            if (this != &rhs)
            {
               if (is_engaged)
               {
                  reinterpret_cast<value_type*>(data.data())->~value_type(); // NOLINT
               }

               is_engaged = rhs.is_engaged;
               rhs.is_engaged = false;

               if (is_engaged)
               {
                  new (data.data()) value_type{
                     std::move(*reinterpret_cast<value_type*>(rhs.data.data()))}; // NOLINT
               }
            }

            return *this;
         }

         alignas(any_) std::array<std::byte, sizeof(value_type)> data;
         bool is_engaged{false};
      };

      template <trivially_destructible any_>
      struct maybe_storage<any_>
      {
         using value_type = any_;

         maybe_storage() = default;
         maybe_storage(const value_type& value) : is_engaged{true}
         {
            if (is_engaged)
            {
               new (&data) value_type{value};
            }
         }
         maybe_storage(value_type&& value) : is_engaged{true}
         {
            if (is_engaged)
            {
               new (&data) value_type{std::move(value)};
            }
         }

         alignas(any_) std::array<std::byte, sizeof(value_type)> data;
         bool is_engaged{false};
      };
   } // namespace detail

   // clang-format off
   template <class any_>
      requires(!std::is_reference_v<any_>) 
   class maybe;
   // clang-format on

   template <>
   class maybe<void>
   {
   };

   // clang-format off
   template <class any_>
      requires(!std::is_reference_v<any_>) 
   class maybe
   // clang-format on
   {
   public:
      using value_type = typename detail::maybe_storage<any_>::value_type;

   public:
      constexpr maybe() = default;
      constexpr maybe(const value_type& value) : m_storage{value} {}
      constexpr maybe(value_type&& value) : m_storage{std::move(value)} {}
      constexpr maybe(maybe<void>) {}

      constexpr auto operator->() const -> const value_type*
      {
         assert(has_value());

         return reinterpret_cast<const value_type*>(&m_storage.data); // NOLINT
      }
      constexpr auto operator->() -> value_type*
      {
         assert(has_value());

         return reinterpret_cast<value_type*>(m_storage.data.data()); // NOLINT
      }

      constexpr auto operator*() const& -> const value_type& { return value(); }
      constexpr auto operator*() & -> value_type& { return value(); }
      constexpr auto operator*() const&& -> value_type&& { return std::move(value()); }
      constexpr auto operator*() && -> value_type&& { return std::move(value()); }

      constexpr auto value() const& -> const value_type&
      {
         assert(has_value());

         return *reinterpret_cast<const value_type*>(m_storage.data.data()); // NOLINT
      }
      constexpr auto value() & -> value_type&
      {
         assert(has_value());

         return *reinterpret_cast<value_type*>(m_storage.data.data()); // NOLINT
      }

      constexpr auto value() const&& -> const value_type&&
      {
         assert(has_value());

         return std::move(*reinterpret_cast<value_type*>(m_storage.data.data())); // NOLINT
      }
      constexpr auto value() && -> value_type&&
      {
         assert(has_value());

         return std::move(*reinterpret_cast<value_type*>(m_storage.data.data())); // NOLINT
      }

      constexpr auto value_or(
         std::convertible_to<value_type> auto&& default_value) const& -> value_type
      {
         return has_value()
            ? value()
            : static_cast<value_type>(std::forward<decltype(default_value)>(default_value));
      }
      constexpr auto value_or(std::convertible_to<value_type> auto&& default_value) && -> value_type
      {
         return has_value()
            ? value()
            : static_cast<value_type>(std::forward<decltype(default_value)>(default_value));
      }

      [[nodiscard]] constexpr auto has_value() const noexcept -> bool
      {
         return m_storage.is_engaged;
      }
      constexpr operator bool() const noexcept { return has_value(); }

      // clang-format off
      constexpr auto map(const std::invocable<value_type> auto& fun) const& 
         requires std::copyable<value_type>
      {
         using result_type = std::invoke_result_t<decltype(fun), value_type>;
         if (!has_value())
         {
            return maybe<result_type>{};
         }
         else
         {
            return maybe<result_type>(fun(value()));
         }
      }
      constexpr auto map(const std::invocable<value_type> auto& fun) &
         requires std::movable<value_type>
      {
         using result_type = std::invoke_result_t<decltype(fun), value_type>;
         if (!has_value())
         {
            return maybe<result_type>{};
         }
         else
         {
            return maybe<result_type>(fun(std::move(value())));
         }
      }

      constexpr auto map(const std::invocable<value_type> auto& fun) && 
      {
         using result_type = std::invoke_result_t<decltype(fun), value_type>;
         if (!has_value())
         {
            return maybe<result_type>{};
         }
         else
         {
            return maybe<result_type>(fun(std::move(value())));
         }
      }

      constexpr auto operator>>=(const std::invocable<value_type> auto& fun) const& 
         requires std::copyable<value_type>
      {
         return map(fun);
      };
      
      constexpr auto operator>>=(const std::invocable<value_type> auto& fun) &
         requires std::movable<value_type>
      {
         return map(fun);
      };

      constexpr auto operator>>=(const std::invocable<value_type> auto& fun) &&
      {
         return map(fun);
      };
      // clang-format on

   private:
      detail::maybe_storage<value_type> m_storage{};
   };

   template <class any_>
   auto to_maybe(any_&& value) -> maybe<std::remove_reference_t<any_>>
   {
      return {std::forward<any_>(value)};
   }

   template <class any_ = void>
   auto to_maybe()
   {
      return maybe<any_>{};
   }

   namespace monad
   {
      inline static const auto none = to_maybe();
   } // namespace monad
} // namespace core
