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
#include <type_traits>

namespace ESL
{
   template <typename any_>
   concept trivially_copyable = std::is_trivially_copyable_v<any_>;

   template <typename any_>
   concept trivially_default_constructible = std::is_trivially_default_constructible_v<any_>;

   template <typename any_>
   concept trivial = trivially_copyable<any_> && trivially_default_constructible<any_>;

   template <class B>
   concept boolean = std::movable<std::remove_cvref_t<B>>&& requires(
      const std::remove_reference_t<B>& b1, const std::remove_reference_t<B>& b2, const bool a )
   {
      {
         b1
      }
      ->std::convertible_to<bool>;
      {
         !b1
      }
      ->std::convertible_to<bool>;
      {
         b1&& b2
      }
      ->std::same_as<bool>;
      {
         b1&& a
      }
      ->std::same_as<bool>;
      {
         a&& b2
      }
      ->std::same_as<bool>;
      {
         b1 || b2
      }
      ->std::same_as<bool>;
      {
         b1 || a
      }
      ->std::same_as<bool>;
      {
         a || b2
      }
      ->std::same_as<bool>;
      {
         b1 == b2
      }
      ->std::convertible_to<bool>;
      {
         b1 == a
      }
      ->std::convertible_to<bool>;
      {
         a == b2
      }
      ->std::convertible_to<bool>;
      {
         b1 != b2
      }
      ->std::convertible_to<bool>;
      {
         b1 != a
      }
      ->std::convertible_to<bool>;
      {
         a != b2
      }
      ->std::convertible_to<bool>;
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
} // namespace ESL
