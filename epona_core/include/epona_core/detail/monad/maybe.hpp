#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>

namespace core
{
   template <class any_>
   class maybe;

   template <>
   class maybe<void>
   {
   };

   template <class any_>
   class maybe
   {
   public:
      using value_type = any_;

   public:
      constexpr maybe() : is_init{false} {}
      constexpr maybe(const value_type& value) : val{value}, is_init{true} {}
      constexpr maybe(value_type&& value) : val{std::move(value)}, is_init{true} {}
      constexpr maybe(maybe<void>) : is_init{false} {}
      constexpr maybe(const maybe& other) : is_init{other.is_init}
      {
         if (is_init)
         {
            new (&val) value_type{other.val};
         }
      }
      constexpr maybe(maybe&& other) : is_init{other.is_init}
      {
         if (is_init)
         {
            new (&val) value_type{std::move(other.val)};
         }

         other.destroy();
      }
      ~maybe() { destroy(); }

      constexpr maybe& operator=(const value_type& value)
      {
         if (is_init)
         {
            destroy();
         }

         is_init = true;
         val = value;

         return *this;
      }
      constexpr maybe& operator=(value_type&& value)
      {
         if (is_init)
         {
            destroy();
         }

         is_init = true;
         val = std::move(value);

         return *this;
      }

      constexpr const value_type* operator->() const
      {
         assert(is_init);

         return &val;
      }
      constexpr value_type* operator->()
      {
         assert(is_init);

         return &val;
      }

      constexpr const value_type& operator*() const&
      {
         assert(is_init);

         return val;
      }
      constexpr value_type& operator*() &
      {
         assert(is_init);

         return val;
      }
      constexpr value_type&& operator*() &&
      {
         assert(is_init);

         return std::move(val);
      }

      constexpr const value_type& value() const&
      {
         assert(is_init);

         return val;
      }
      constexpr value_type& value() &
      {
         assert(is_init);

         return val;
      }

      constexpr const value_type&& value() const&&
      {
         assert(is_init);

         return std::move(val);
      }
      constexpr value_type&& value() &&
      {
         assert(is_init);

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

      [[nodiscard]] constexpr bool has_value() const noexcept { return is_init; }
      constexpr operator bool() const noexcept { return is_init; }

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
         if (is_init)
         {
            val.~value_type();
         }
      }

   private:
      union
      {
         value_type val;
      };

      bool is_init;
   };

   template <class any_>
   auto to_maybe(any_&& value)
   {
      return maybe<any_>{std::forward<any_>(value)};
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
