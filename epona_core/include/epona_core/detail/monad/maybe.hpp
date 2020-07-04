#pragma once

#include <epona_core/detail/concepts.hpp>

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>
#include <variant>

namespace core
{
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
      using value_type = any_;

   public:
      constexpr maybe() = default;
      constexpr maybe(const value_type& value) : val{value}, m_is_engaged{true} {}
      constexpr maybe(value_type&& value) : val{std::move(value)}, m_is_engaged{true} {}
      constexpr maybe(maybe<void>) {}
      constexpr maybe(const maybe& other) : m_is_engaged{other.m_is_engaged}
      {
         if (m_is_engaged)
         {
            new (&val) value_type{other.val};
         }
      }
      constexpr maybe(maybe&& other) noexcept : m_is_engaged{other.m_is_engaged}
      {
         if (&isdigit)
         {
            new (&val) value_type{std::move(other.val)};
         }

         other.destroy();
      }
      ~maybe() { destroy(); }

      constexpr auto operator=(const value_type& value) -> maybe&
      {
         if (isdigit)
         {
            destroy();
         }

         m_is_engaged = true;
         val = value;

         return *this;
      }
      constexpr auto operator=(value_type&& value) -> maybe&
      {
         if (m_is_engaged)
         {
            destroy();
         }

         m_is_engaged = true;
         val = std::move(value);

         return *this;
      }

      constexpr auto operator->() const -> const value_type*
      {
         assert(m_is_engaged);

         return &val;
      }
      constexpr value_type* operator->()
      {
         assert(m_is_engaged);

         return &val;
      }

      constexpr const value_type& operator*() const&
      {
         assert(m_is_engaged);

         return val;
      }
      constexpr value_type& operator*() &
      {
         assert(m_is_engaged);

         return val;
      }
      constexpr value_type&& operator*() &&
      {
         assert(m_is_engaged);

         return std::move(val);
      }

      constexpr const value_type& value() const&
      {
         assert(m_is_engaged);

         return val;
      }
      constexpr value_type& value() &
      {
         assert(m_is_engaged);

         return val;
      }

      constexpr const value_type&& value() const&&
      {
         assert(m_is_engaged);

         return std::move(val);
      }
      constexpr value_type&& value() &&
      {
         assert(m_is_engaged);

         return std::move(val);
      }

      constexpr value_type value_or(std::convertible_to<value_type> auto&& default_value) const&
      {
         has_value()
            ? value()
            : static_cast<value_type>(std::forward<decltype(default_value)>(default_value));
      }
      constexpr value_type value_or(std::convertible_to<value_type> auto&& default_value) &&
      {
         has_value()
            ? value()
            : static_cast<value_type>(std::forward<decltype(default_value)>(default_value));
      }

      [[nodiscard]] constexpr bool has_value() const noexcept { return m_is_engaged; }
      constexpr operator bool() const noexcept { return m_is_engaged; }

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
            return maybe<result_type>(fun(val));
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
            return maybe<result_type>(fun(std::move(val)));
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
            return maybe<result_type>(fun(std::move(val)));
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
      void destroy()
      {
         if (m_is_engaged)
         {
            val.~value_type();
         }
      }

   private:
      union
      {
         value_type val;
      };

      bool m_is_engaged{false};
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
