#pragma once

#include "epona_core/details/monad/option.hpp"

#include <cassert>
#include <utility>

namespace core
{
   namespace monad
   {
      template <typename any_>
      struct left_t
      {
         any_ value;
      };

      template <typename any_>
      struct right_t
      {
         any_ value;
      };

      template <typename any_>
      constexpr auto to_left(const any_& any) -> left_t<any_>
      {
         return {any};
      }

      template <std::movable any_>
      constexpr auto to_left(any_&& any) -> left_t<any_>
      {
         return {std::move(any)};
      }

      template <typename any_>
      constexpr auto to_right(const any_& any) -> right_t<any_>
      {
         return {any};
      }

      template <std::movable any_>
      constexpr auto to_right(any_&& any) -> right_t<any_>
      {
         return {std::move(any)};
      }
   } // namespace monad

   template <typename left_, typename right_>
   class either
   {
   public:
      using left_type = left_;
      using right_type = right_;

   public:
      constexpr either(const monad::left_t<left_type>& left) : left_val{left.value}, is_left{true}
      {}
      constexpr either(monad::left_t<left_type>&& left) :
         left_val{std::move(left.value)}, is_left{true}
      {}
      constexpr either(const monad::right_t<right_type>& right) :
         right_val{right.value}, is_left{false}
      {}
      constexpr either(monad::right_t<right_type>&& right) :
         right_val{std::move(right.value)}, is_left{false}
      {}
      constexpr either(const either& other) : is_left{other.is_left}
      {
         if (is_left)
         {
            new (&left_val) left_type{other.left};
         }
         else
         {
            new (&right_val) right_type{other.right};
         }
      }
      constexpr either(either&& other) : is_left{other.is_left}
      {
         if (is_left)
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

      constexpr either& operator=(const monad::left_t<left_type>& left)
      {
         destroy();

         is_left = true;
         new (&left) left_type{left};

         return *this;
      }
      constexpr either& operator=(monad::left_t<left_type>&& left)
      {
         destroy();

         is_left = true;
         new (&left) left_type{std::move(left)};

         return *this;
      }
      constexpr either& operator=(const monad::right_t<right_type>& right)
      {
         destroy();

         is_left = false;
         new (&right) right_type{right};

         return *this;
      }
      constexpr either& operator=(monad::right_t<right_type>&& right)
      {
         destroy();

         is_left = false;
         new (&right) right_type{std::move(right)};

         return *this;
      }
      constexpr either& operator=(const either& rhs)
      {
         if (this != &rhs)
         {
            destroy();

            is_left = rhs.is_left;
            if (is_left)
            {
               new (&left_val) left_type{rhs.left};
            }
            else
            {
               new (&right_val) right_type{rhs.right};
            }
         }

         return *this;
      }
      constexpr either& operator=(either&& rhs)
      {
         if (this != &rhs)
         {
            destroy();

            is_left = rhs.is_left;
            if (is_left)
            {
               new (&left_val) left_type{std::move(rhs.left)};
            }
            else
            {
               new (&right_val) right_type{std::move(rhs.right)};
            }
         }

         return *this;
      }

      constexpr option<left_type> left()
      {
         if (is_left)
         {
            return {left_val};
         }
         else
         {
            return {};
         }
      }

      constexpr option<right_type> right()
      {
         if (is_left)
         {
            return {};
         }
         else
         {
            return {right_val};
         }
      }

   private:
      void destroy()
      {
         if (is_left)
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

      bool is_left;

   public:
      template <std::copyable inner_left_ = left_type, std::copyable inner_right_ = right_type>
      constexpr auto join() const -> std::common_type_t<inner_left_, inner_right_>
      {
         return is_left ? left_val : right_val;
      }

      template <std::movable inner_left_ = left_type, std::movable inner_right_ = right_type>
      constexpr auto join() && -> std::common_type_t<inner_left_, inner_right_>
      {
         return is_left ? std::move(left_val) : std::move(right_val);
      }

      template <std::movable inner_left_ = left_type, std::movable inner_right_ = right_type>
      constexpr auto join() & -> std::common_type_t<inner_left_, inner_right_>
      {
         return is_left ? std::move(left_val) : std::move(right_val);
      }

      constexpr auto join(const std::invocable<left_type> auto& left_fun,
         const std::invocable<right_type> auto& right_fun) const
         -> decltype(is_left ? left_fun(left_val) : right_fun(right_val))
      {
         return is_left ? left_fun(left_val) : right_fun(right_val);
      }

      // clang-format off
      constexpr auto join(const std::invocable<left_type> auto& left_fun,
         const std::invocable<right_type> auto& right_fun) & 
         -> decltype(is_left ? left_fun(std::move(left_val)) : right_fun(std::move(right_val))) 
         requires std::movable<left_type>&& std::movable<right_type>
      {
         return is_left ? left_fun(std::move(left_val)) : right_fun(std::move(right_val));
      }

      template <std::copyable inner_right_ = right_type>
      constexpr auto left_map(const std::invocable<left_type> auto& fun) const& 
         -> either<decltype(fun(left_val)), inner_right_> requires std::copyable<left_type>
      {
         if (is_left)
         {
            return monad::left_t<decltype(fun(left_val))>{fun(left_val)};
         }
         else
         {
            return monad::right_t<inner_right_>{right_val};
         }
      }

      template <std::movable inner_right_ = right_type>
      constexpr auto left_map(const std::invocable<left_type> auto& fun) &&
         -> either<decltype(fun(std::move(left_val))), inner_right_>
      {
         if (is_left)
         {
            return monad::left_t<decltype(fun(std::move(left_val)))>{fun(std::move(left_val))};
         }
         else
         {
            return monad::right_t<inner_right_>{std::move(right_val)};
         }
      }

      template <std::movable inner_right_ = right_type>
      constexpr auto left_map(const std::invocable<left_type> auto& fun) &
         -> either<decltype(fun(std::move(left_val))), inner_right_> requires std::movable<left_type>
      {
         if (is_left)
         {
            return monad::left_t<decltype(fun(std::move(left_val)))>{fun(std::move(left_val))};
         }
         else
         {
            return monad::right_t<inner_right_>{std::move(right_val)};
         }
      }
      
      template<std::copyable inner_left_ = left_type>
      constexpr auto right_map(const std::invocable<right_type> auto& fun) const&
         -> either<inner_left_, decltype(fun(right_val))> requires std::copyable<right_type>
      {
         if (is_left)
         {
            return monad::left_t<inner_left_>{left_val};
         }
         else
         {
            return monad::right_t<decltype(fun(right_val))>{fun(right_val)};
         }
      }

      template<std::movable inner_left_ = left_type>
      constexpr auto right_map(const std::invocable<right_type> auto& fun) &&
         -> either<inner_left_, decltype(fun(std::move(right_val)))>
      {
         if (is_left)
         {
            return monad::left_t<inner_left_>{std::move(left_val)};
         }
         else
         {
            return monad::right_t<decltype(fun(std::move(right_val)))>{fun(std::move(right_val))};
         }
      }

      template<std::movable inner_left_ = left_type>
      constexpr auto right_map(const std::invocable<right_type> auto& fun) &
         -> either<inner_left_, decltype(fun(std::move(right_val)))> requires std::movable<right_type>
      {
         if (is_left)
         {
            return monad::left_t<inner_left_>{std::move(left_val)};
         }
         else
         {
            return monad::right_t<decltype(fun(std::move(right_val)))>{fun(std::move(right_val))};
         }
      }
      // clang-format on
   };

   namespace monad
   {
      // clang-format off
      template <class error_, class fun_, class... args_>
         requires std::invocable<fun_, args_...> 
      constexpr auto try_wrap(const fun_& fun, args_&&... args) 
         -> either<std::invoke_result_t<fun_, args_...>, error_>
      {
         using ret_type = std::invoke_result_t<fun_, args_...>;
         try
         {
            return monad::left_t<ret_type>(fun(std::forward<args_>(args)...));
         }
         catch (const error_& e)
         {
            return monad::right_t<error_>(e);
         }
      }
      // clang-format on
   } // namespace monad
} // namespace core
