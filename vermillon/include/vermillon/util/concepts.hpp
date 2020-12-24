/**
 * MIT License
 *
 * Copyright (c) 2020 Wmbat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>

namespace cacao
{
   // clang-format off
   template <typename any_>
   concept trivially_default_constructible = std::is_trivially_default_constructible_v<any_>;

   template <typename any_>
   concept trivially_copyable = std::is_trivially_copyable_v<any_>;

   template <typename any_>
   concept trivially_destructible = std::is_trivially_destructible_v<any_>;

   template <typename any_>
   concept trivial = 
      trivially_default_constructible<any_> &&
      trivially_copyable<any_> &&  
      trivially_destructible<any_>;

   template <class B>
   concept boolean = std::movable<std::remove_cvref_t<B>> && 
      requires( 
         const std::remove_reference_t<B>& b1, 
         const std::remove_reference_t<B>& b2, 
         const bool a )
      {
         { b1 } -> std::convertible_to<bool>;
         { !b1 } -> std::convertible_to<bool>;
         { b1 && b2 } -> std::same_as<bool>;
         { b1 && a } -> std::same_as<bool>;
         { a && b2 } -> std::same_as<bool>;
         { b1 || b2 } -> std::same_as<bool>;
         { b1 || a } -> std::same_as<bool>;
         { a || b2 } -> std::same_as<bool>;
         { b1 == b2 } -> std::convertible_to<bool>;
         { b1 == a } -> std::convertible_to<bool>;
         { a == b2 } -> std::convertible_to<bool>;
         { b1 != b2 } -> std::convertible_to<bool>;
         { b1 != a } -> std::convertible_to<bool>;
         { a != b2 } -> std::convertible_to<bool>;
      };

   template <class T>
   concept totally_ordered = std::equality_comparable<T> && 
   requires( const std::remove_reference_t<T>& a, const std::remove_reference_t<T>& b )
   {
      { a < b } -> boolean;
      { a > b } -> boolean;
      { a <= b } -> boolean;
      { a >= b } -> boolean;
   };

   template <class allocator_>
   concept allocator = 
      std::default_initializable<allocator_> && 
      requires(allocator_ a, typename allocator_::value_type* p, std::size_t n)
      {
         { a.allocate(n) } -> std::same_as<typename allocator_::value_type*>;
         { a.deallocate(p, n)} -> std::same_as<void>;
      };
   // clang-format on
} // namespace cacao
