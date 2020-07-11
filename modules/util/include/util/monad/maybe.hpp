#pragma once

#include <util/concepts.hpp>

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>
#include <variant>

namespace util
{
   namespace detail
   {
      consteval auto max(size_t size) noexcept -> size_t { return size; }
      consteval auto max(size_t first, size_t second, std::unsigned_integral auto... rest) noexcept
         -> size_t
      {
         return first >= second ? max(first, rest...) : max(second, rest...);
      }
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
      template <class type_, class dummy_ = void>
      struct storage
      {
         using value_type = type_;

         constexpr storage() = default;
         constexpr storage(const value_type& value) : is_engaged{true}
         {
            if (is_engaged)
            {
               new (data.data()) value_type{value};
            }
         }
         constexpr storage(value_type&& value) : is_engaged{true}
         {
            if (is_engaged)
            {
               new (data.data()) value_type{std::move(value)};
            }
         }
         constexpr storage(const storage& rhs) : is_engaged{rhs.is_engaged}
         {
            if (is_engaged)
            {
               new (data.data()) value_type{rhs.value()};
            }
         }
         constexpr storage(storage&& rhs) noexcept : is_engaged{rhs.is_engaged}
         {
            if (is_engaged)
            {
               new (data.data()) value_type{std::move(rhs.value())};
            }
         }
         ~storage()
         {
            if (is_engaged)
            {
               value().~value_type();
            }
         }

         constexpr auto operator=(const storage& rhs) -> storage&
         {
            if (this != &rhs)
            {
               if (is_engaged)
               {
                  value().~value_type();
               }

               is_engaged = rhs.is_engaged;

               if (is_engaged)
               {
                  new (data.data()) value_type{rhs.value()};
               }
            }

            return *this;
         }
         constexpr auto operator=(storage&& rhs) noexcept -> storage&
         {
            if (this != &rhs)
            {
               if (is_engaged)
               {
                  value().~value_type();
               }

               is_engaged = rhs.is_engaged;
               rhs.is_engaged = false;

               if (is_engaged)
               {
                  new (data.data()) value_type{std::move(rhs.value())};
               }
            }

            return *this;
         }

         constexpr auto pointer() -> value_type*
         {
            return reinterpret_cast<value_type*>(data.data()); // NOLINT
         }
         constexpr auto pointer() const -> const value_type*
         {
            return reinterpret_cast<const value_type*>(data.data()); // NOLINT
         }

         constexpr auto value() & -> value_type&
         {
            return *reinterpret_cast<value_type*>(data.data()); // NOLINT
         }
         constexpr auto value() const& -> const value_type&
         {
            return *reinterpret_cast<const value_type*>(data.data()); // NOLINT
         }
         constexpr auto value() && -> value_type&&
         {
            return std::move(*reinterpret_cast<value_type*>(data.data())); // NOLINT
         }
         constexpr auto value() const&& -> const value_type&&
         {
            return std::move(*reinterpret_cast<value_type*>(data.data())); // NOLINT
         }

         alignas(any_) std::array<std::byte, sizeof(value_type)> data;
         bool is_engaged{false};
      };

      template <class type_>
      struct storage<type_, std::enable_if_t<trivial<type_>>>
      {
         using value_type = any_;

         constexpr storage() = default;
         constexpr storage(const value_type& value) : is_engaged{true}
         {
            if (is_engaged)
            {
               new (&data) value_type{value};
            }
         }
         constexpr storage(value_type&& value) : is_engaged{true}
         {
            if (is_engaged)
            {
               new (&data) value_type{std::move(value)};
            }
         }

         constexpr auto pointer() -> value_type*
         {
            return reinterpret_cast<value_type*>(data.data()); // NOLINT
         }
         constexpr auto pointer() const -> const value_type*
         {
            return reinterpret_cast<const value_type*>(data.data()); // NOLINT
         }

         constexpr auto value() & -> value_type&
         {
            return *reinterpret_cast<value_type*>(data.data()); // NOLINT
         }
         constexpr auto value() const& -> const value_type&
         {
            return *reinterpret_cast<const value_type*>(data.data()); // NOLINT
         }
         constexpr auto value() && -> value_type&&
         {
            return std::move(*reinterpret_cast<value_type*>(data.data())); // NOLINT
         }
         constexpr auto value() const&& -> const value_type&&
         {
            return std::move(*reinterpret_cast<value_type*>(data.data())); // NOLINT
         }

         alignas(any_) std::array<std::byte, sizeof(value_type)> data;
         bool is_engaged{false};
      };

   public:
      using value_type = any_;

   public:
      constexpr maybe() = default;
      constexpr maybe(const value_type& value) : m_storage{value} {}
      constexpr maybe(value_type&& value) : m_storage{std::move(value)} {}
      constexpr maybe(maybe<void>) {}

      constexpr auto operator->() const -> const value_type*
      {
         assert(has_value());

         return m_storage.pointer();
      }
      constexpr auto operator->() -> value_type*
      {
         assert(has_value());

         return m_storage.pointer();
      }

      constexpr auto operator*() & -> value_type& { return m_storage.value(); }
      constexpr auto operator*() const& -> const value_type& { return m_storage.value(); }
      constexpr auto operator*() && -> value_type&& { return std::move(m_storage.value()); }
      constexpr auto operator*() const&& -> const value_type&&
      {
         return std::move(m_storage.value());
      }

      constexpr auto value() const& -> const value_type&
      {
         assert(has_value());

         return m_storage.value();
      }
      constexpr auto value() & -> value_type&
      {
         assert(has_value());

         return m_storage.value();
      }
      constexpr auto value() const&& -> const value_type&&
      {
         assert(has_value());

         return std::move(m_storage.value());
      }
      constexpr auto value() && -> value_type&&
      {
         assert(has_value());

         return std::move(m_storage.value());
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
      storage<value_type> m_storage{};
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
} // namespace util
