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
      struct error
      {
         any_ val;
      };

      template <class any_>
      struct value
      {
         any_ val;
      };

      template <class any_>
      constexpr error<std::remove_reference_t<any_>> to_error(any_&& value)
      {
         return {.val = std::forward<std::remove_reference_t<any_>>(value)};
      }

      template <class any_>
      constexpr value<std::remove_reference_t<any_>> to_value(any_&& value)
      {
         return {.val = std::forward<std::remove_reference_t<any_>>(value)};
      }
   } // namespace monad

   // clang-format off
   template <class error_, class value_> 
      requires (!(std::is_reference_v<error_> || std::is_reference_v<value_>))
   class result
   // clang-format on
   {
   public:
      using error_type = error_;
      using value_type = value_;

   private:
      template <class any_>
      using error_map_result = std::invoke_result_t<any_, error_type>;

      template <class fun_>
      using error_map_either = result<error_map_result<fun_>, value_type>;

      template <class any_>
      using value_map_result = std::invoke_result_t<any_, value_type>;

      template <class fun_>
      using value_map_either = result<error_type, value_map_result<fun_>>;

   public:
      constexpr result(const monad::error<error_type>& error) : m_error{error.val}, m_is_init{false}
      {}
      constexpr result(monad::error<error_type>&& error) :
         m_error{std::move(error.val)}, m_is_init{false}
      {}
      constexpr result(const monad::value<value_type>& value) : m_value{value.val}, m_is_init{true}
      {}
      constexpr result(monad::value<value_type>&& value) :
         m_value{std::move(value.val)}, m_is_init{true}
      {}
      constexpr result(const result& rhs) : m_is_init{rhs.m_is_init}
      {
         if (m_is_init)
         {
            new (&m_error) error_type{rhs.m_error};
         }
         else
         {
            new (&m_value) value_type{rhs.m_value};
         }
      }
      constexpr result(result&& rhs) : m_is_init{rhs.m_is_init}
      {
         if (m_is_init)
         {
            new (&m_error) error_type{std::move(rhs.m_error)};
         }
         else
         {
            new (&m_value) value_type{std::move(rhs.m_value)};
         }
      }
      constexpr ~result() { destroy(); }

   private:
      union
      {
         error_type m_error;
         value_type m_value;
      };

      bool m_is_init;

   public:
      constexpr bool has_value() const { return m_is_init; }
      constexpr operator bool() const { return has_value(); }

      constexpr auto error() const& -> maybe<error_type>
      {
         if (has_value())
         {
            return to_maybe(error_type{m_error});
         }
         else
         {
            return monad::none;
         }
      }
      constexpr auto error() & -> maybe<error_type> requires std::movable<error_type>
      {
         if (has_value())
         {
            return to_maybe(std::move(m_error));
         }
         else
         {
            return monad::none;
         }
      }
      constexpr auto error() && -> maybe<error_type>
      {
         if (has_value())
         {
            return to_maybe(std::move(m_error));
         }
         else
         {
            return monad::none;
         }
      }

      constexpr auto value() const& -> maybe<value_type>
      {
         if (has_value())
         {
            return monad::none;
         }
         else
         {
            return to_maybe(value_type{m_value});
         }
      }
      constexpr auto value() & -> maybe<value_type> requires std::movable<value_type>
      {
         if (has_value())
         {
            return monad::none;
         }
         else
         {
            return to_maybe(std::move(m_value));
         }
      }
      constexpr auto right() && -> maybe<value_type>
      {
         if (has_value())
         {
            return monad::none;
         }
         else
         {
            return to_maybe(std::move(m_value));
         }
      }

      // clang-format off
      constexpr auto error_map(const std::invocable<error_type> auto& fun) const&
         -> error_map_either<decltype(fun)> requires std::copyable<error_type>
      {
         if (!has_value())
         {
            return monad::to_error(fun(m_error));
         }
         else
         {
            return monad::value<value_type>{m_value};
         }
      }

      constexpr auto error_map(const std::invocable<error_type> auto& fun) &
         -> error_map_either<decltype(fun)> requires std::movable<error_type>
      {
         if (!has_value())
         {
            return monad::to_error(fun(std::move(m_error)));
         }
         else
         {
            return monad::to_value(std::move(m_value));
         }
      }

      constexpr auto error_map(const std::invocable<error_type> auto& fun) &&
         -> error_map_either<decltype(fun)>
      {
         if (!has_value())
         {
            return monad::to_error(fun(std::move(m_error)));
         }
         else
         {
            return monad::to_value(std::move(m_value));
         }
      }

      constexpr auto value_map(const std::invocable<value_type> auto& fun) const&
         -> value_map_either<decltype(fun)> requires std::copyable<value_type>
      {
         if (!has_value())
         {
            return monad::to_error(m_error);
         }
         else
         {
            return monad::to_value(fun(m_value));
         }
      }
      
      constexpr auto value_map(const std::invocable<value_type> auto& fun) &
         -> value_map_either<decltype(fun)> requires std::movable<value_type>
      {
         if (!has_value())
         {
            return monad::to_error(std::move(m_error));
         }
         else
         {
            return monad::to_value(fun(std::move(m_value)));
         }
      }

      constexpr auto value_map(const std::invocable<value_type> auto& fun) &&
         -> value_map_either<decltype(fun)>
      {
         if (!has_value())
         {
            return monad::to_error(std::move(m_error));
         }
         else
         {
            return monad::to_value(fun(std::move(m_value)));
         }
      }

      template <std::copyable inner_error_ = error_type, std::copyable inner_value_ = value_type>
      constexpr auto join() const -> std::common_type_t<inner_error_, inner_value_>
      {
         return !has_value() ? m_error : m_value;
      }

      template <std::movable inner_error_ = error_type, std::movable inner_value_ = value_type>
      constexpr auto join() && -> std::common_type_t<inner_error_, inner_value_>
      {
         return !has_value() ? std::move(m_error) : std::move(m_value);
      }

      template <std::movable inner_error_ = error_type, std::movable inner_value_ = value_type>
      constexpr auto join() & -> std::common_type_t<inner_error_, inner_value_>
      {
         return !has_value() ? std::move(m_error) : std::move(m_value);
      }

      constexpr auto join(const std::invocable<error_type> auto& left_fun,
         const std::invocable<value_type> auto& right_fun) const
         -> decltype(!has_value() ? left_fun(m_error) : right_fun(m_value))
      {
         return !has_value() ? left_fun(m_error) : right_fun(m_value);
      }

      // clang-format off
      constexpr auto join(const std::invocable<error_type> auto& err_fun,
         const std::invocable<value_type> auto& val_fun) & 
         -> decltype(!has_value() ? err_fun(std::move(m_error)) : val_fun(std::move(m_value))) 
         requires std::movable<error_type> && std::movable<value_type>
      {
         return !has_value() ? left_fun(std::move(m_error)) : right_fun(std::move(m_value));
      }
      // clang-format on

   private:
      constexpr void destroy()
      {
         if (m_is_init)
         {
            m_error.~error_type();
         }
         else
         {
            m_value.~value_type();
         }
      }
   };
} // namespace core
