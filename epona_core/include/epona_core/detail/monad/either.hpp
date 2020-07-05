#pragma once

#include <epona_core/detail/monad/maybe.hpp>

#include <algorithm>
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

      template <class any_>
      constexpr left<std::remove_reference_t<any_>> to_left(any_&& value)
      {
         return left<std::remove_reference_t<any_>>{std::forward<any_>(value)};
      }

      template <class any_>
      constexpr right<std::remove_reference_t<any_>> to_right(any_&& value)
      {
         return {.val = std::forward<std::remove_reference_t<any_>>(value)};
      }
   } // namespace monad

   // clang-format off
   template <class left_, class right_> 
      requires (!(std::is_reference_v<left_> || std::is_reference_v<right_>))
   class either
   // clang-format on
   {
   public:
      using left_type = left_;
      using right_type = right_;

   private:
      template <class any_>
      using left_map_result = std::invoke_result_t<any_, left_type>;

      template <class fun_>
      using left_map_either = either<left_map_result<fun_>, right_type>;

      template <class any_>
      using right_map_result = std::invoke_result_t<any_, right_type>;

      template <class fun_>
      using right_map_either = either<left_type, right_map_result<fun_>>;

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

      constexpr auto left() const& -> maybe<left_type>
      {
         if (is_left_val)
         {
            return to_maybe(left_type{left_val});
         }
         else
         {
            return to_maybe();
         }
      }
      constexpr auto left() & -> maybe<left_type> requires std::movable<left_type>
      {
         if (is_left())
         {
            return to_maybe(std::move(left_val));
         }
         else
         {
            return to_maybe();
         }
      }
      constexpr auto left() && -> maybe<left_type>
      {
         if (is_left_val)
         {
            return to_maybe(std::move(left_val));
         }
         else
         {
            return to_maybe();
         }
      }
      constexpr auto right() const& -> maybe<right_type>
      {
         if (is_left())
         {
            return to_maybe();
         }
         else
         {
            return to_maybe(right_type{right_val});
         }
      }
      constexpr auto right() & -> maybe<right_type> requires std::movable<left_type>
      {
         if (is_left())
         {
            return to_maybe();
         }
         else
         {
            if constexpr (std::movable<right_type>)
            {
               return to_maybe(std::move(right_val));
            }
            else
            {
               return to_maybe(right_val);
            }
         }
      }
      constexpr auto right() && -> maybe<right_type>
      {
         if (is_left_val)
         {
            return to_maybe();
         }
         else
         {
            return to_maybe(std::move(right_val));
         }
      }

      template <class any_>
      constexpr auto left_or(any_&& default_value) const& -> left_type
         requires std::constructible_from<left_type, any_>
      {
         if (is_left())
         {
            return left_val;
         }
         else
         {
            return static_cast<left_type>(std::forward<any_>(default_value));
         }
      }
      template <class any_>
      constexpr auto left_or(any_&& default_value) & -> left_type
         requires std::constructible_from<left_type, any_>
      {
         if (is_left())
         {
            if constexpr (std::movable<left_type>)
            {
               return std::move(left_val);
            }
            else
            {
               return left_val;
            }
         }
         else
         {
            return static_cast<left_type>(std::forward<any_>(default_value));
         }
      }
      template <class any_>
      constexpr auto left_or(any_&& default_value) && -> left_type
         requires std::constructible_from<left_type, any_>
      {
         if (is_left())
         {
            return std::move(left_val);
         }
         else
         {
            return static_cast<left_type>(std::forward<any_>(default_value));
         }
      }

      template <class any_>
      constexpr auto right_or(any_&& default_value) const& -> right_type
         requires std::constructible_from<right_type, any_>
      {
         if (is_left())
         {
            return static_cast<right_type>(std::forward<any_>(default_value));
         }
         else
         {
            return right_val;
         }
      }
      template <class any_>
      constexpr auto right_or(any_&& default_value) & -> right_type
         requires std::constructible_from<right_type, any_>
      {
         if (is_left())
         {
            return static_cast<right_type>(std::forward<any_>(default_value));
         }
         else
         {
            if constexpr (std::movable<right_type>)
            {
               return std::move(right_val);
            }
            else
            {
               return right_val;
            }
         }
      }
      template <class any_>
      constexpr auto right_or(any_&& default_value) && -> right_type
         requires std::constructible_from<right_type, any_>
      {
         if (is_left())
         {
            return static_cast<right_type>(std::forward<any_>(default_value));
         }
         else
         {
            return std::move(right_val);
         }
      }

      // clang-format off
      constexpr auto left_map(const std::invocable<left_type> auto& fun) const& 
         -> left_map_either<decltype(fun)> requires std::copyable<left_type>
      {
         if (is_left())
         {
            return monad::to_left(fun(left_val));
         }
         else
         {
            return monad::right<right_type>{right_val};
         }
      }

      constexpr auto left_map(const std::invocable<left_type> auto& fun) &
         -> left_map_either<decltype(fun)> requires std::movable<left_type>
      {
         if (is_left_val)
         {
            return monad::to_left(fun(std::move(left_val)));
         }
         else
         {
            return monad::to_right(std::move(right_val));
         }
      }

      constexpr auto left_map(const std::invocable<left_type> auto& fun) &&
         -> left_map_either<decltype(fun)>
      {
         if (is_left_val)
         {
            return monad::to_left(fun(std::move(left_val)));
         }
         else
         {
            return monad::to_right(std::move(right_val));
         }
      }

      constexpr auto operator<<=(const std::invocable<left_type> auto& fun) const& 
         -> left_map_either<decltype(fun)> requires std::copyable<left_type>
      {
         return left_map(fun);
      };

      constexpr auto operator<<=(const std::invocable<left_type> auto& fun) &
         -> left_map_either<decltype(fun)> requires std::movable<left_type>
      {
         return left_map(fun);
      };

      constexpr auto operator<<=(const std::invocable<left_type> auto& fun) &&
         -> left_map_either<decltype(fun)> requires std::movable<left_type>
      {
         return left_map(fun);
      };

      constexpr auto right_map(const std::invocable<right_type> auto& fun) const&
         -> right_map_either<decltype(fun)> requires std::copyable<right_type>
      {
         if (is_left_val)
         {
            return monad::left<left_type>(left_val);
         }
         else
         {
            return monad::to_right(fun(right_val));
         }
      }
      
      constexpr auto right_map(const std::invocable<right_type> auto& fun) &
         -> right_map_either<decltype(fun)> requires std::movable<right_type>
      {
         if (is_left_val)
         {
            return monad::to_left(std::move(left_val));
         }
         else
         {
            return monad::to_right(fun(std::move(right_val)));
         }
      }

      constexpr auto right_map(const std::invocable<right_type> auto& fun) &&
         -> either<left_type, right_map_result<decltype(fun)>>
      {
         if (is_left_val)
         {
            return monad::to_left(std::move(left_val));
         }
         else
         {
            return monad::to_right(fun(std::move(right_val)));
         }
      }

      constexpr auto operator>>=(const std::invocable<right_type> auto& fun) const&
         -> right_map_either<decltype(fun)> requires std::copyable<right_type>
      {
         return right_map(fun); 
      }

      constexpr auto operator>>=(const std::invocable<right_type> auto& fun) &
         -> right_map_either<decltype(fun)> requires std::movable<right_type>
      {
         return right_map(fun); 
      }

      constexpr auto operator>>=(const std::invocable<right_type> auto& fun) &&
         -> right_map_either<decltype(fun)>
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
         try
         {
            return monad::to_right(fun(std::forward<args_>(args)...));
         }
         catch (const error_& e)
         {
            return monad::left{e};
         }
      }
      // clang-format on  
   } // namespace monad
} // namespace core
