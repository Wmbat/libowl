#pragma once

#include <util/monad/maybe.hpp>

#include <algorithm>
#include <cassert>
#include <utility>

namespace util
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
      constexpr auto to_error(any_&& err) -> error<std::remove_reference_t<any_>>
      {
         return {.val = std::forward<std::remove_reference_t<any_>>(err)};
      }

      template <class any_>
      constexpr auto to_value(any_&& val) -> value<std::remove_reference_t<any_>>
      {
         return {.val = std::forward<std::remove_reference_t<any_>>(val)};
      }
   } // namespace monad

   // clang-format off
   template <class error_, class value_> 
      requires (!(std::is_reference_v<error_> || std::is_reference_v<value_>))
   class result
   // clang-format on
   {
      template <class first_, class second_, class dummy_1_ = void, class dummy_2_ = void>
      struct storage
      {
         using error_type = first_;
         using value_type = second_;

         constexpr storage() = default;
         constexpr storage(const monad::error<error_type>& error)
         {
            new (bytes.data()) error_type{error.val};
         }
         constexpr storage(monad::error<error_type>&& error)
         {
            new (bytes.data()) error_type{std::move(error.val)};
         }
         constexpr storage(const monad::value<value_type>& value) : has_value{true}
         {
            new (bytes.data()) value_type{value.val};
         }
         constexpr storage(monad::value<value_type>&& value) : has_value{true}
         {
            new (bytes.data()) value_type{std::move(value.val)};
         }
         constexpr storage(const storage& rhs) : has_value{rhs.has_value}
         {
            if (has_value)
            {
               new (bytes.data()) value_type{rhs.value()};
            }
            else
            {
               new (bytes.data()) error_type{rhs.error()};
            }
         }
         constexpr storage(storage&& rhs) noexcept
         {
            if (has_value)
            {
               new (bytes.data()) value_type{std::move(rhs.value())};
            }
            else
            {
               new (bytes.data()) error_type{std::move(rhs.error())};
            }
         }
         ~storage()
         {
            if (has_value)
            {
               value().~value_type();
            }
            else
            {
               error().~error_type();
            }
         }

         constexpr auto operator=(const storage& rhs) -> storage&
         {
            if (this != &rhs)
            {
               if (has_value)
               {
                  value().~value_type();
               }
               else
               {
                  error().~error_type();
               }

               has_value = rhs.has_value;

               if (has_value)
               {
                  new (bytes.data()) value_type{rhs.value()};
               }
               else
               {
                  new (bytes.data()) error_type{rhs.error()};
               }
            }

            return *this;
         }
         constexpr auto operator=(storage&& rhs) noexcept -> storage&
         {
            if (this != &rhs)
            {
               if (has_value)
               {
                  value().~value_type();
               }
               else
               {
                  error().~error_type();
               }

               has_value = rhs.has_value;

               if (has_value)
               {
                  new (bytes.data()) value_type{std::move(rhs.value())};
               }
               else
               {
                  new (bytes.data()) error_type{std::move(rhs.error())};
               }
            }

            return *this;
         }

         constexpr auto error() & noexcept -> error_type&
         {
            return *reinterpret_cast<error_type*>(bytes.data()); // NOLINT
         }
         constexpr auto error() const& noexcept -> const error_type&
         {
            return *reinterpret_cast<const error_type*>(bytes.data()); // NOLINT
         }
         constexpr auto error() && noexcept -> error_type&&
         {
            return std::move(*reinterpret_cast<error_type*>(bytes.data())); // NOLINT
         }
         constexpr auto error() const&& noexcept -> const error_type&&
         {
            return std::move(*reinterpret_cast<error_type*>(bytes.data())); // NOLINT
         }

         constexpr auto value() & noexcept -> value_type&
         {
            return *reinterpret_cast<value_type*>(bytes.data()); // NOLINT
         }
         constexpr auto value() const& noexcept -> const value_type&
         {
            return *reinterpret_cast<const value_type*>(bytes.data()); // NOLINT
         }
         constexpr auto value() && noexcept -> value_type&&
         {
            return std::move(*reinterpret_cast<value_type*>(bytes.data())); // NOLINT
         }
         constexpr auto value() const&& noexcept -> const value_type&&
         {
            return std::move(*reinterpret_cast<value_type*>(bytes.data())); // NOLINT
         }

         alignas(detail::max(alignof(error_type), alignof(value_type)))
            std::array<std::byte, detail::max(sizeof(error_type), sizeof(value_type))> bytes{};
         bool has_value{false};
      };

      template <class first_, class second_>
      struct storage<first_, second_, std::enable_if_t<trivial<first_>>,
         std::enable_if_t<trivial<second_>>>
      {
         using error_type = first_;
         using value_type = second_;

         constexpr storage(const monad::error<error_type>& error)
         {
            new (bytes.data()) error_type{error.val};
         }
         constexpr storage(monad::error<error_type>&& error)
         {
            new (bytes.data()) error_type{std::move(error.val)};
         }
         constexpr storage(const monad::value<value_type>& value) : has_value{true}
         {
            new (bytes.data()) value_type{value.val};
         }
         constexpr storage(monad::value<value_type>&& value) : has_value{true}
         {
            new (bytes.data()) value_type{std::move(value.val)};
         }

         constexpr auto error() & noexcept -> error_type&
         {
            return *reinterpret_cast<error_type*>(bytes.data()); // NOLINT
         }
         constexpr auto error() const& noexcept -> const error_type&
         {
            return *reinterpret_cast<const error_type*>(bytes.data()); // NOLINT
         }
         constexpr auto error() && noexcept -> error_type&&
         {
            return std::move(*reinterpret_cast<error_type*>(bytes.data())); // NOLINT
         }
         constexpr auto error() const&& noexcept -> const error_type&&
         {
            return std::move(*reinterpret_cast<error_type*>(bytes.data())); // NOLINT
         }

         constexpr auto value() & noexcept -> value_type&
         {
            return *reinterpret_cast<value_type*>(bytes.data()); // NOLINT
         }
         constexpr auto value() const& noexcept -> const value_type&
         {
            return *reinterpret_cast<const value_type*>(bytes.data()); // NOLINT
         }
         constexpr auto value() && noexcept -> value_type&&
         {
            return std::move(*reinterpret_cast<value_type*>(bytes.data())); // NOLINT
         }
         constexpr auto value() const&& noexcept -> const value_type&&
         {
            return std::move(*reinterpret_cast<value_type*>(bytes.data())); // NOLINT
         }

         alignas(detail::max(alignof(error_type), alignof(value_type)))
            std::array<std::byte, detail::max(sizeof(error_type), sizeof(value_type))> bytes{};
         bool has_value{false};
      };

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
      constexpr result(const monad::error<error_type>& error) : m_storage{error} {}
      constexpr result(monad::error<error_type>&& error) : m_storage{std::move(error)} {}
      constexpr result(const monad::value<value_type>& value) : m_storage{value} {}
      constexpr result(monad::value<value_type>&& value) : m_storage{std::move(value)} {}

   private:
      storage<error_type, value_type> m_storage;

   public:
      [[nodiscard]] constexpr auto has_value() const -> bool { return m_storage.has_value; }
      constexpr operator bool() const { return has_value(); }

      constexpr auto error() const& -> maybe<error_type>
      {
         if (has_value())
         {
            return to_maybe();
         }
         else
         {
            return to_maybe(error_type{m_storage.error()});
         }
      }
      constexpr auto error() & -> maybe<error_type> requires std::movable<error_type>
      {
         if (has_value())
         {
            return to_maybe();
         }
         else
         {
            return to_maybe(std::move(m_storage.error()));
         }
      }
      constexpr auto error() && -> maybe<error_type>
      {
         if (has_value())
         {
            return to_maybe();
         }
         else
         {
            return to_maybe(std::move(m_storage.error()));
         }
      }

      constexpr auto value() const& -> maybe<value_type>
      {
         if (has_value())
         {
            return to_maybe(value_type{m_storage.value()});
         }
         else
         {
            return to_maybe();
         }
      }
      constexpr auto value() & -> maybe<value_type> requires std::movable<value_type>
      {
         if (has_value())
         {
            return to_maybe(std::move(m_storage.value()));
         }
         else
         {
            return to_maybe();
         }
      }
      constexpr auto value() && -> maybe<value_type>
      {
         if (has_value())
         {
            return to_maybe(std::move(m_storage.value()));
         }
         else
         {
            return to_maybe();
         }
      }

      // clang-format off
      constexpr auto error_map(const std::invocable<error_type> auto& fun) const&
         -> error_map_either<decltype(fun)> requires std::copyable<error_type>
      {
         if (!has_value())
         {
            return monad::to_error(fun(m_storage.error()));
         }
         else
         {
            return monad::value<value_type>{m_storage.value()};
         }
      }

      constexpr auto error_map(const std::invocable<error_type> auto& fun) &
         -> error_map_either<decltype(fun)> requires std::movable<error_type>
      {
         if (!has_value())
         {
            return monad::to_error(fun(std::move(m_storage.error())));
         }
         else
         {
            return monad::to_value(std::move(m_storage.value()));
         }
      }

      constexpr auto error_map(const std::invocable<error_type> auto& fun) &&
         -> error_map_either<decltype(fun)>
      {
         if (!has_value())
         {
            return monad::to_error(fun(std::move(m_storage.error())));
         }
         else
         {
            return monad::to_value(std::move(m_storage.value()));
         }
      }

      constexpr auto value_map(const std::invocable<value_type> auto& fun) const&
         -> value_map_either<decltype(fun)> requires std::copyable<value_type>
      {
         if (!has_value())
         {
            return monad::to_error(m_storage.error());
         }
         else
         {
            return monad::to_value(fun(m_storage.value()));
         }
      }
      
      constexpr auto value_map(const std::invocable<value_type> auto& fun) &
         -> value_map_either<decltype(fun)> requires std::movable<value_type>
      {
         if (!has_value())
         {
            return monad::to_error(std::move(m_storage.error()));
         }
         else
         {
            return monad::to_value(fun(std::move(m_storage.value())));
         }
      }

      constexpr auto value_map(const std::invocable<value_type> auto& fun) &&
         -> value_map_either<decltype(fun)>
      {
         if (!has_value())
         {
            return monad::to_error(std::move(m_storage.error()));
         }
         else
         {
            return monad::to_value(fun(std::move(m_storage.value())));
         }
      }

      template <std::copyable inner_error_ = error_type, std::copyable inner_value_ = value_type>
      constexpr auto join() const -> std::common_type_t<inner_error_, inner_value_>
      {
         return !has_value() ? m_storage.error() : m_storage.value();
      }

      template <std::movable inner_error_ = error_type, std::movable inner_value_ = value_type>
      constexpr auto join() && -> std::common_type_t<inner_error_, inner_value_>
      {
         return !has_value() ? std::move(m_storage.error()) : std::move(m_storage.value());
      }

      template <std::movable inner_error_ = error_type, std::movable inner_value_ = value_type>
      constexpr auto join() & -> std::common_type_t<inner_error_, inner_value_>
      {
         return !has_value() ? std::move(m_storage.error()) : std::move(m_storage.value());
      }

      constexpr auto join(const std::invocable<error_type> auto& left_fun,
         const std::invocable<value_type> auto& right_fun) const
         -> decltype(!has_value() ? left_fun(m_storage.error()) : right_fun(m_storage.value()))
      {
         return !has_value() ? left_fun(m_storage.error()) : right_fun(m_storage.value());
      }

      // clang-format off
      constexpr auto join(const std::invocable<error_type> auto& err_fun,
         const std::invocable<value_type> auto& val_fun) & 
         -> decltype(!has_value() ? 
               err_fun(std::move(m_storage.error())) : 
               val_fun(std::move(m_storage.value()))) 
         requires std::movable<error_type> && std::movable<value_type>
      {
         return 
            !has_value() ? 
               err_fun(std::move(m_storage.error())) : 
               val_fun(std::move(m_storage.value()));
      }
      // clang-format on
   };
} // namespace util
