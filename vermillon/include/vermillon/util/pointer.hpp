#pragma once

#include <vermillon/util/assert.hpp>

#include <concepts>
#include <system_error>
#include <type_traits>
#include <utility>

namespace cacao
{
   /**
    * @brief Simple construct to signal that a pointer will not be null
    */
   template <typename Any>
   class non_null
   {
   public:
      static_assert(std::is_convertible_v<decltype(std::declval<Any>() != nullptr), bool>,
                    "Any cannot be compared to nullptr.");

      template <std::convertible_to<Any> T>
      constexpr non_null(T&& t) : m_ptr{std::forward<T>(t)} // NOLINT
      {
         EXPECT(m_ptr != nullptr);
      }

      template <typename = std::enable_if_t<!std::is_same<std::nullptr_t, Any>::value>>
      constexpr non_null(Any any) : m_ptr{std::move(any)}
      {
         EXPECT(m_ptr != nullptr);
      }

      template <std::convertible_to<Any> T>
      constexpr non_null(const non_null<T>& other) : non_null{other.get()}
      {}

      non_null(const non_null& other) = default;
      non_null(non_null&& other) noexcept = default;
      ~non_null() = default;

      auto operator=(const non_null& other) -> non_null& = default;
      auto operator=(non_null&& other) noexcept -> non_null& = default;

      [[nodiscard]] constexpr auto get() const
         -> std::conditional_t<std::is_copy_constructible<Any>::value, Any, const Any&>
      {
         ENSURE(m_ptr != nullptr);

         return m_ptr;
      }

      constexpr operator Any() const { return get(); }
      constexpr decltype(auto) operator->() const { return get(); }
      constexpr decltype(auto) operator*() const { return *get(); }

      // prevents compilation when someone attempts to assign a null pointer constant
      non_null(std::nullptr_t) = delete;
      auto operator=(std::nullptr_t) -> non_null& = delete;

      // unwanted operators...pointers only point to single objects!
      auto operator++() -> non_null& = delete;
      auto operator--() -> non_null& = delete;
      auto operator++(int) -> const non_null = delete;
      auto operator--(int) -> const non_null = delete;
      auto operator+=(std::ptrdiff_t) -> non_null& = delete;
      auto operator-=(std::ptrdiff_t) -> non_null& = delete;

      void operator[](std::ptrdiff_t) const = delete;

   private:
      Any m_ptr;
   };

   template <class Any>
   auto make_non_null(Any&& any) noexcept
   {
      return non_null<std::remove_cv_t<std::remove_reference_t<Any>>>{std::forward<Any>(any)};
   }

   template <class First, class Second>
   auto operator==(const non_null<First>& lhs,
                   const non_null<Second>& rhs) noexcept(noexcept(lhs.get() == rhs.get()))
      -> decltype(lhs.get() == rhs.get())
   {
      return lhs.get() == rhs.get();
   }
} // namespace cacao

namespace std
{
   template <typename Any>
   struct hash<cacao::non_null<Any>>
   {
      auto operator()(const cacao::non_null<Any>& value) const -> std::size_t
      {
         return hash<Any>{}(value.get());
      }
   };
} // namespace std
