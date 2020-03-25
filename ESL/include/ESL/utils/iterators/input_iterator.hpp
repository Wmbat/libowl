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

#include <cstdint>
#include <iterator>

namespace ESL
{
   template <class type_>
   class input_iterator
   {
   public:
      using iterator_category = std::input_iterator_tag;
      using self_type = input_iterator;
      using value_type = type_;
      using reference = type_&;
      using const_reference = type_ const&;
      using pointer = type_*;
      using const_pointer = type_ const*;
      using difference_type = std::ptrdiff_t;

   public:
      constexpr explicit input_iterator( ) = default;
      constexpr explicit input_iterator( pointer p_type ) noexcept : p_type( p_type ) {}

      constexpr bool operator==( self_type rhs ) const noexcept = default;

      constexpr const_reference operator*( ) const noexcept
      {
         assert( p_type != nullptr && "Cannot derefence a nullptr" );

         return *p_type;
      }
      constexpr const_pointer operator->( ) const noexcept
      {
         assert( p_type != nullptr && "Cannot derefence a nullptr" );

         return p_type;
      };

      constexpr self_type& operator++( ) noexcept
      {
         ++p_type;

         return *this;
      }

      constexpr self_type operator++( int ) noexcept
      {
         self_type it = *this;
         ++*this;

         return it;
      }

      constexpr void swap( self_type& rhs ) noexcept { std::swap( p_type, rhs.get_ptr( ) ); }

   private:
      pointer p_type{ nullptr };
   };
} // namespace ESL
