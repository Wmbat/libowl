#pragma once

#include "epona_core/details/monad/maybe.hpp"

#include <cassert>
#include <utility>

namespace core
{
   namespace monad
   {
      template <class any_>
      struct left
      {
         any_ val;
      };

      template <class any_>
      struct right
      {
         any_ val;
      };
   } // namespace monad

   template <class left_, class right_>
   class either
   {
   public:
      using left_type = left_;
      using right_type = right_;

   private:
      template <class any_>
      using left_map_result = std::invoke_result_t<any_, left_type>;

      template <class any_>
      using right_map_result = std::invoke_result_t<any_, right_type>;

   public:
      constexpr either(const monad::left<left_type>& left) : left_val{left.val}, is_left_val{true}
      {}
      constexpr either(monad::left<left_type>&& left) :
         left_val{std::move(left.val)}, is_left_val{true}
      {}
      constexpr either(const monad::right<right_type>& right) :
         right_val{right.val}, is_left_val{false}
      {}
      constexpr either(monad::right<right_type>&& right) :
         right_val{std::move(right.val)}, is_left_val{false}
      {}
      constexpr either(const either& other) : is_left_val{other.is_left}
      {
         if (is_left_val)
         {
            new (&left_val) left_type{other.left};
         }
         else
         {
            new (&right_val) right_type{other.right};
         }
      }
      constexpr either(either&& other) : is_left_val{other.is_left}
      {
         if (is_left_val)
         {
            new (&left_val) left_type{std::move(other.left)};
         }
         else
         {
            new (&right_val) right_type{std::move(other.right)};
         }

         other.destroy();
      }
      ~either() { destroy(); }

      constexpr bool is_left() const { return is_left_val; }

      constexpr maybe<left_type> left() const& requires std::copyable<right_type>
      {
         if (is_left_val)
         {
            return to_maybe(left_val);
         }
         else
         {
            return monad::none;
         }
      }
      constexpr maybe<left_type> left() & requires std::movable<left_type>
      {
         if (is_left_val)
         {
            return to_maybe(std::move(left_val));
         }
         else
         {
            return monad::none;
         }
      }
      constexpr maybe<left_type> left() &&
      {
         if (is_left_val)
         {
            return to_maybe(std::move(left_val));
         }
         else
         {
            return monad::none;
         }
      }

      constexpr maybe<right_type> right() const& requires std::copyable<right_type>
      {
         if (is_left_val)
         {
            return monad::none;
         }
         else
         {
            return to_maybe(right_val);
         }
      }
      constexpr maybe<right_type> right() & requires std::movable<right_type>
      {
         if (is_left_val)
         {
            return monad::none;
         }
         else
         {
            return to_maybe(std::move(right_val));
         }
      }
      constexpr maybe<right_type> right() &&
      {
         if (is_left_val)
         {
            return monad::none;
         }
         else
         {
            return to_maybe(std::move(right_val));
         }
      }

      // clang-format off
      template<std::copyable inner_left_ = left_type>
      constexpr auto right_map(const std::invocable<right_type> auto& fun) const&
         -> either<inner_left_, right_map_result<decltype(fun)>> requires std::copyable<right_type>
      {
         if (is_left_val)
         {
            return monad::left<inner_left_>{left_val};
         }
         else
         {
            return monad::right<decltype(fun(right_val))>{fun(right_val)};
         }
      }
      
      template<std::movable inner_left_ = left_type>
         constexpr auto right_map(const std::invocable<right_type> auto& fun) &
         -> either<inner_left_, right_map_result<decltype(fun)>> requires std::movable<right_type>
         {
            if (is_left_val)
            {
               return monad::left<inner_left_>{std::move(left_val)};
            }
            else
            {
               return monad::right<decltype(fun(std::move(right_val)))>{fun(std::move(right_val))};
            }
         }

      template<std::movable inner_left_ = left_type>
      constexpr auto right_map(const std::invocable<right_type> auto& fun) &&
         -> either<inner_left_, right_map_result<decltype(fun)>>
      {
         if (is_left_val)
         {
            return monad::left<inner_left_>{std::move(left_val)};
         }
         else
         {
            return monad::right<decltype(fun(std::move(right_val)))>{fun(std::move(right_val))};
         }
      }

      constexpr auto operator>>=(const std::invocable<right_type> auto& fun) const&
         -> either<left_type, right_map_result<decltype(fun)>> requires std::copyable<right_type>
      {
         return right_map(fun); 
      }

      constexpr auto operator>>=(const std::invocable<right_type> auto& fun) &
         -> either<left_type, right_map_result<decltype(fun)>> requires std::movable<right_type>
      {
         return right_map(fun); 
      }

      constexpr auto operator>>=(const std::invocable<right_type> auto& fun) &&
         -> either<left_type, right_map_result<decltype(fun)>>
      {
         return right_map(fun); 
      }
      // clang-format on

   private:
      void destroy()
      {
         if (is_left_val)
         {
            left_val.~left_type();
         }
         else
         {
            right_val.~right_type();
         }
      }

   private:
      union
      {
         left_type left_val;
         right_type right_val;
      };

      bool is_left_val;

   public:
      template <std::copyable inner_left_ = left_type, std::copyable inner_right_ = right_type>
      constexpr auto join() const -> std::common_type_t<inner_left_, inner_right_>
      {
         return is_left() ? left_val : right_val;
      }

      template <std::movable inner_left_ = left_type, std::movable inner_right_ = right_type>
      constexpr auto join() && -> std::common_type_t<inner_left_, inner_right_>
      {
         return is_left() ? std::move(left_val) : std::move(right_val);
      }

      template <std::movable inner_left_ = left_type, std::movable inner_right_ = right_type>
      constexpr auto join() & -> std::common_type_t<inner_left_, inner_right_>
      {
         return is_left() ? std::move(left_val) : std::move(right_val);
      }

      constexpr auto join(const std::invocable<left_type> auto& left_fun,
         const std::invocable<right_type> auto& right_fun) const
         -> decltype(is_left() ? left_fun(left_val) : right_fun(right_val))
      {
         return is_left() ? left_fun(left_val) : right_fun(right_val);
      }

      // clang-format off
      constexpr auto join(const std::invocable<left_type> auto& left_fun,
         const std::invocable<right_type> auto& right_fun) & 
         -> decltype(is_left() ? left_fun(std::move(left_val)) : right_fun(std::move(right_val))) 
         requires std::movable<left_type>&& std::movable<right_type>
      {
         return is_left() ? left_fun(std::move(left_val)) : right_fun(std::move(right_val));
      }

      template <std::copyable inner_right_ = right_type>
      constexpr auto left_map(const std::invocable<left_type> auto& fun) const& 
         -> either<decltype(fun(left_val)), inner_right_> requires std::copyable<left_type>
      {
         if (is_left())
         {
            return monad::left<decltype(fun(left_val))>{fun(left_val)};
         }
         else
         {
            return monad::right<inner_right_>{right_val};
         }
      }

      template <std::movable inner_right_ = right_type>
      constexpr auto left_map(const std::invocable<left_type> auto& fun) &&
         -> either<decltype(fun(std::move(left_val))), inner_right_>
      {
         if (is_left_val)
         {
            return monad::left<decltype(fun(std::move(left_val)))>{fun(std::move(left_val))};
         }
         else
         {
            return monad::right<inner_right_>{std::move(right_val)};
         }
      }

      template <std::movable inner_right_ = right_type>
      constexpr auto left_map(const std::invocable<left_type> auto& fun) &
         -> either<decltype(fun(std::move(left_val))), inner_right_> requires std::movable<left_type>
      {
         if (is_left_val)
         {
            return monad::left<decltype(fun(std::move(left_val)))>{fun(std::move(left_val))};
         }
         else
         {
            return monad::right<inner_right_>{std::move(right_val)};
         }
      }
     
      constexpr auto operator<<=(const std::invocable<left_type> auto& fun) const& 
         -> either<decltype(fun(left_val)), right_type> requires std::copyable<left_type>
      {
         return left_map(fun);
      };

      constexpr auto operator<<=(const std::invocable<left_type> auto& fun) &&
         -> either<decltype(fun(std::move(left_val))), right_type> requires std::movable<left_type>
      {
         return left_map(fun);
      };

      constexpr auto operator<<=(const std::invocable<left_type> auto& fun) &
         -> either<decltype(fun(std::move(left_val))), right_type> requires std::movable<left_type>
      {
         return left_map(fun);
      };
      // clang-format on
   };

   namespace monad
   {
      // clang-format off
      template <class error_, class fun_, class... args_>
         requires std::invocable<fun_, args_...>
      auto try_wrap(const fun_& fun, args_&&... args) 
         -> either<error_, std::invoke_result_t<fun_, args_...>>
      {
         using result_t = std::invoke_result_t<fun_, args_...>;
         try
         {
            return monad::right<result_t>{.val = fun(std::forward<args_>(args)...)};
         }
         catch (const error_& e)
         {
            return monad::left{.val = e};
         }
      }
      // clang-format on  
   } // namespace monad
} // namespace core
