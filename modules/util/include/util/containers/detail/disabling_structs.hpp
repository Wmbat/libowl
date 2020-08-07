/**
 * @file disabling_structs.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date 19th of July, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <type_traits>

namespace util::detail
{
   template <class any_, bool = std::is_copy_constructible_v<any_>>
   struct disable_copy_constructor
   {
      constexpr disable_copy_constructor() = default;
      constexpr disable_copy_constructor(const disable_copy_constructor&) = delete;
      constexpr disable_copy_constructor(disable_copy_constructor&&) noexcept = default;
      constexpr ~disable_copy_constructor() = default;

      constexpr auto operator=(const disable_copy_constructor&)
         -> disable_copy_constructor& = default;
      constexpr auto operator=(disable_copy_constructor&&) noexcept
         -> disable_copy_constructor& = default;
   };

   template <class T>
   struct disable_copy_constructor<T, true>
   {
   };

   template <class T, bool = std::is_copy_assignable_v<T>>
   struct disable_copy_assignment
   {
      constexpr disable_copy_assignment() = default;
      constexpr disable_copy_assignment(const disable_copy_assignment&) = default;
      constexpr disable_copy_assignment(disable_copy_assignment&&) noexcept = default;
      constexpr ~disable_copy_assignment() = default;

      constexpr auto operator=(const disable_copy_assignment&) -> disable_copy_assignment& = delete;
      constexpr auto operator=(disable_copy_assignment&&) noexcept
         -> disable_copy_assignment& = default;
   };

   template <class T>
   struct disable_copy_assignment<T, true>
   {
   };

   template <class T, bool = std::is_move_constructible_v<T>>
   struct disable_move_constructor
   {
      constexpr disable_move_constructor() = default;
      constexpr disable_move_constructor(const disable_move_constructor&) = default;
      constexpr disable_move_constructor(disable_move_constructor&&) noexcept = delete;
      constexpr ~disable_move_constructor() = default;

      constexpr auto operator=(const disable_move_constructor&)
         -> disable_move_constructor& = default;
      constexpr auto operator=(disable_move_constructor&&) noexcept
         -> disable_move_constructor& = default;
   };

   template <class T>
   struct disable_move_constructor<T, true>
   {
   };

   template <class T, bool = std::is_move_assignable_v<T>>
   struct disable_move_assignment
   {
      constexpr disable_move_assignment() = default;
      constexpr disable_move_assignment(const disable_move_assignment&) = default;
      constexpr disable_move_assignment(disable_move_assignment&&) noexcept = default;
      constexpr ~disable_move_assignment() = default;

      constexpr auto operator=(const disable_move_assignment&)
         -> disable_move_assignment& = default;
      constexpr auto operator=(disable_move_assignment &&) -> disable_move_assignment& = delete;
   };

   template <class T>
   struct disable_move_assignment<T, true>
   {
   };
} // namespace util
