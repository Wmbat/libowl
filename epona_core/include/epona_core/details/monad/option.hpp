#pragma once

#include <cassert>
#include <concepts>
#include <utility>

namespace core
{
   template <typename any_>
   class option;

   template <>
   class option<void>
   {
   };

   template <typename any_>
   class option
   {
   public:
      using value_type = any_;

   public:
      constexpr option() : is_init{false} {}
      constexpr option(const value_type& value) : val{value}, is_init{true} {}
      constexpr option(value_type&& value) : val{std::move(value)}, is_init{true} {}
      constexpr option(option<void>) : is_init{false} {}
      constexpr option(const option& other) : is_init{other.is_init}
      {
         if (is_init)
         {
            new (&val) value_type{other.val};
         }
      }
      constexpr option(option&& other) : is_init{other.is_init}
      {
         if (is_init)
         {
            new (&val) value_type{std::move(other.val)};
         }

         other.destroy();
      }
      ~option() { destroy(); }

      constexpr option& operator=(const option& rhs)
      {
         if (this != &rhs)
         {
            destroy();

            is_init = rhs.is_init;
            new (&val) value_type{rhs.val};
         }

         return *this;
      }

      constexpr option& operator=(option&& rhs)
      {
         if (this != &rhs)
         {
            destroy();

            is_init = rhs.is_init;
            new (&val) value_type{std::move(rhs.val)};
         }

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

      [[nodiscard]] constexpr bool has_value() const noexcept { return is_init; }
      constexpr operator bool() const noexcept { return is_init; }

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

   public:
      // clang-format off
      constexpr auto map(const std::invocable<value_type> auto& fun) const& 
         -> option<decltype(fun(val))> requires std::copyable<value_type>
      {
         if (!has_value())
         {
            return {};
         }
         else
         {
            return option(fun(val));
         }
      }

      constexpr auto map(const std::invocable<value_type> auto& fun) && 
         -> option<decltype(fun(std::move(val)))>
      {
         if (!has_value())
         {
            return {};
         }
         else
         {
            return option(fun(std::move(val)));
         }
      }

      // clang-format on
   };
} // namespace core
