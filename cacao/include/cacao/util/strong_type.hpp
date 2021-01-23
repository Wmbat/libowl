#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace cacao
{
   // clang-format off
   template <typename any_, typename parameter, template <typename> class... utils_>
   class strong_type final : public utils_<strong_type<any_, parameter, utils_...>>...
   {
   public:
      using value_type = any_;

   public:
      constexpr strong_type() 
         noexcept(std::is_nothrow_default_constructible_v<value_type>) 
         requires std::default_initializable<value_type> = default;

      constexpr strong_type(const value_type& value) 
         noexcept(std::is_nothrow_copy_constructible_v<value_type>) 
         requires std::copy_constructible<value_type> 
         : m_value{value}
      {}

      constexpr strong_type(value_type&& value) 
         noexcept(std::is_nothrow_move_assignable_v<value_type>)
         requires std::move_constructible<value_type>
         : m_value{std::move(value)}
      {}

      constexpr auto value() noexcept -> value_type&
      {
         return m_value;
      }
      constexpr auto value() const noexcept -> const value_type&
      {
         return m_value;
      }

      constexpr operator strong_type<value_type&, parameter>()
      {
         return strong_type<value_type&, parameter>{m_value};
      }

      struct argument
      {
         constexpr argument() = default;
         constexpr argument(argument const&) = delete;
         constexpr argument(argument&&) = delete;
         constexpr ~argument() = default;

         constexpr auto operator=(argument const&) -> argument& = delete;
         constexpr auto operator=(argument&&) -> argument& = delete;

         constexpr auto operator=(value_type&& value) const -> strong_type // NOLINT
         {
            return strong_type{std::forward<value_type>(value)};
         }

         constexpr auto operator=(auto&& value) const -> strong_type // NOLINT
         {
            return strong_type{std::forward<decltype(value)>(value)};
         }
    };

   private:
      value_type m_value;
   };
   // clang-format on

   namespace detail
   {
      template <typename any_, template <typename> class crtp_type_>
      struct crtp
      {
         constexpr auto underlying() -> any_& { return static_cast<any_&>(*this); }
         constexpr auto underlying() const -> any_ const&
         {
            return static_cast<any_ const&>(*this);
         }
      };
   } // namespace detail

   template <typename any_>
   struct pre_incrementable : detail::crtp<any_, pre_incrementable>
   {
      constexpr auto operator++() -> any_&
      {
         ++this->underlying().value();
         return this->underlying();
      }
   };

   template <typename any_>
   struct post_incrementable : detail::crtp<any_, post_incrementable>
   {
      constexpr auto operator++(int) -> any_ { return this->underlying().value()++; }
   };

   template <typename any_>
   struct incrementable : pre_incrementable<any_>, post_incrementable<any_>
   {
      using pre_incrementable<any_>::operator++;
      using post_incrementable<any_>::operator++;
   };

   template <typename any_>
   struct pre_decrementable : detail::crtp<any_, pre_decrementable>
   {
      constexpr auto operator--() -> any_&
      {
         --this->underlying().value();
         return this->underlying();
      }
   };

   template <typename any_>
   struct post_decrementable : detail::crtp<any_, post_decrementable>
   {
      constexpr auto operator--(int) -> any_ { return this->underlying().value()--; }
   };

   template <typename any_>
   struct decrementable : pre_decrementable<any_>, post_decrementable<any_>
   {
      using pre_decrementable<any_>::operator--;
      using post_decrementable<any_>::operator--;
   };

   template <typename any_>
   struct binary_addable : detail::crtp<any_, binary_addable>
   {
      constexpr auto operator+(const any_& rhs) const
      {
         return any_{this->underlying().value() + rhs.value()};
      }

      constexpr auto operator+=(const any_& rhs)
      {
         this->underlying().value() += rhs.value();
         return this->underlying();
      }
   };

   template <typename any_>
   struct unary_addable : detail::crtp<any_, unary_addable>
   {
      constexpr auto operator+() const { return any_{+this->undelying().value()}; }
   };

   template <typename any_>
   struct addable : binary_addable<any_>, unary_addable<any_>
   {
      using binary_addable<any_>::operator+;
      using unary_addable<any_>::operator+;
   };

   template <typename any_>
   struct binary_subtractable : detail::crtp<any_, binary_subtractable>
   {
      constexpr auto operator-(const any_& rhs) const
      {
         return any_{this->underlying().value() - rhs.value()};
      }

      constexpr auto operator-=(const any_& rhs)
      {
         this->underlying().value() -= rhs.value();
         return this->underlying();
      }
   };

   template <typename any_>
   struct unary_subtractable : detail::crtp<any_, unary_subtractable>
   {
      constexpr auto operator-() const { return any_{-this->undelying().value()}; }
   };

   template <typename any_>
   struct subtractable : binary_subtractable<any_>, unary_subtractable<any_>
   {
      using binary_subtractable<any_>::operator-;
      using unary_subtractable<any_>::operator-;
   };

   template <typename any_>
   struct multiplicable : detail::crtp<any_, multiplicable>
   {
      auto operator*(const any_& rhs) const -> any_
      {
         return any_{this->underlying().value() * rhs.value()};
      }
      auto operator*=(const any_& rhs) -> any_&
      {
         this->underlying().value() *= rhs.value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct divisible : detail::crtp<Any, divisible>
   {
      auto operator/(const Any& other) const -> Any
      {
         return Any{this->underlying().value() / other.value()};
      }
      auto operator/=(const Any& rhs) -> Any&
      {
         this->underlying().value() /= rhs.value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct modulable : detail::crtp<Any, modulable>
   {
      auto operator%(const Any& rhs) const -> Any
      {
         return Any{this->underlying().value() % rhs.value()};
      }
      auto operator%=(const Any& other) -> Any&
      {
         this->underlying().value() %= other.value();
         return this->underlying();
      }
   };

   template <typename Any>
   struct arithmetic :
      incrementable<Any>,
      decrementable<Any>,
      subtractable<Any>,
      addable<Any>,
      multiplicable<Any>,
      divisible<Any>,
      modulable<Any>
   {
   };

   template <typename Any>
   struct hashable
   {
      static constexpr bool is_hashable = true;
   };

   using count32_t = strong_type<std::uint32_t, struct count32, arithmetic, hashable>;
   using count64_t = strong_type<std::uint64_t, struct count64, arithmetic, hashable>;
   using index_t = strong_type<std::size_t, struct index, arithmetic, hashable>;
   using size_t = strong_type<std::size_t, struct size, arithmetic, hashable>;
} // namespace cacao

namespace std
{
   template <typename Any, typename Parameter, template <typename> class... Skills>
   struct hash<cacao::strong_type<Any, Parameter, Skills...>>
   {
      using strong_type = cacao::strong_type<Any, Parameter, Skills...>;
      using check_hashable = typename std::enable_if_t<strong_type::is_hashable, void>;

      auto operator()(const strong_type& x) const noexcept -> size_t
      {
         static_assert(noexcept(std::hash<Any>()(x.get())), "hash fuction should not throw");

         return std::hash<Any>{}(x.value());
      }
   };
} // namespace std
