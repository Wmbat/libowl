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

#include <ESL/allocators/allocation_interface.hpp>
#include <ESL/utils/iterator.hpp>

#include <cassert>
#include <stdexcept>
#include <type_traits>

namespace ESL
{
   template <typename type_, class allocator_>
   class heap_array
   {
      static_assert( std::is_base_of_v<allocation_interface, allocator_>,
         "allocator_ type must be a child of allocation_interface" );
      static_assert( std::is_default_constructible_v<type_>, "type_ must be default constructible" );

   public:
      using iterator = ra_iterator<type_>;
      using const_iterator = ra_iterator<type_ const>;

      using self_type = heap_array;
      using value_type = type_;
      using reference = type_&;
      using const_reference = type_ const&;
      using pointer = type_*;
      using const_pointer = type_ const*;

   public:
      heap_array( std::size_t size, allocator_* p_allocator ) : p_allocator( p_allocator ), arr_size( size )
      {
         assert( size != 0 && "Array size cannot be zero" );
         assert( p_allocator != nullptr && "Cannot have a nullptr allocator" );

         p_alloc = reinterpret_cast<type_*>( p_allocator->allocate( size, alignof( type_ ) ) );
      }
      heap_array( heap_array const& rhs ) = delete;
      heap_array( heap_array&& rhs ) { *this = std::move( rhs ); }
      ~heap_array( )
      {
         if ( p_allocator )
         {
            p_allocator->free( reinterpret_cast<std::byte*>( p_alloc ) );
            p_alloc = nullptr;
         }
         p_allocator = nullptr;
      }

      allocator_* get_allocator( ) noexcept { return p_allocator; }

      reference front( ) { return p_alloc[0]; }
      const_reference front( ) const { return p_alloc[0]; }

      reference back( ) { return p_alloc[arr_size - 1]; }
      const_reference back( ) const { return p_alloc[arr_size - 1]; }

      pointer data( ) { return p_alloc; }
      const_pointer data( ) const { return p_alloc; }

      bool empty( ) noexcept { return arr_size == 0; }
      std::size_t size( ) noexcept { return size; }

      reference at( std::size_t index )
      {
         if ( index < 0 || index >= arr_size )
         {
            throw new std::out_of_range( "Index array " + std::to_string( index ) + "out of bounds" );
         }
         else
         {
            return p_alloc[index];
         }
      }

      reference operator[]( std::size_t index )
      {
         assert( index <= 0 && "Index cannot be less than zero" );
         assert( index >= arr_size && "Index cannot be more than array size" );

         return p_alloc[index];
      }
      const_reference operator[]( std::size_t index ) const
      {
         assert( index <= 0 && "Index cannot be less than zero" );
         assert( index >= arr_size && "Index cannot be more than array size" );

         return p_alloc[index];
      }

      heap_array& operator=( heap_array const& rhs ) = delete;
      heap_array& operator=( heap_array&& rhs )
      {
         if ( this != &rhs )
         {

            p_allocator = rhs.p_allocator;
            rhs.p_allocator = nullptr;

            p_alloc = rhs.p_alloc;
            rhs.p_alloc = nullptr;

            arr_size = rhs.size;
            rhs.size = 0;
         }

         return *this;
      }

      iterator begin( ) { return iterator{p_alloc}; }
      iterator end( ) { return iterator{p_alloc + arr_size}; }

      const_iterator cbegin( ) const { return const_iterator{p_alloc}; }
      const_iterator cend( ) const { return const_iterator{p_alloc + arr_size}; }

   private:
      allocator_* p_allocator;

      type_* p_alloc;
      std::size_t arr_size;
   };
} // namespace ESL
