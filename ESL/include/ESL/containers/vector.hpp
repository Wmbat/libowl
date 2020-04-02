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

#include <ESL/allocators/allocator_utils.hpp>
#include <ESL/allocators/multipool_allocator.hpp>
#include <ESL/utils/compare.hpp>
#include <ESL/utils/concepts.hpp>
#include <ESL/utils/iterators/random_access_iterator.hpp>

#include <algorithm>
#include <cassert>
#include <compare>
#include <type_traits>

#define TO_TYPE_PTR( ptr ) reinterpret_cast<pointer>( ptr )

namespace ESL
{
   template <class type_, allocator allocator_ = ESL::multipool_allocator>
   class vector
   {
      using is_ptr = std::is_pointer<type_>;

   public:
      using iterator = random_access_iterator<type_>;
      using const_iterator = random_access_iterator<type_ const>;

      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      using self_type = vector;
      using value_type = type_;
      using reference = type_&;
      using const_reference = type_ const&;
      using pointer = type_*;
      using const_pointer = type_ const*;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;

   public:
      explicit vector( allocator_* p_allocator ) noexcept : p_allocator( p_allocator )
      {
         assert( p_allocator != nullptr && "Cannot have a nullptr allocator" );
      }
      explicit vector(
         size_type count, const_reference value, allocator_* p_allocator ) requires std::copyable<value_type> :
         p_allocator( p_allocator ),
         current_capacity( count ),
         current_size( count )
      {
         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         for ( size_type i = 0; i < current_size; ++i )
         {
            new ( TO_BYTE_PTR( p_alloc + i ) ) value_type( value );
         }
      }
      explicit vector( size_type count, allocator_* p_allocator ) requires std::default_initializable<value_type> :
         p_allocator( p_allocator ),
         current_capacity( count ),
         current_size( count )
      {
         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         for ( size_type i = 0; i < current_size; ++i )
         {
            new ( reinterpret_cast<std::byte*>( p_alloc + i ) ) value_type( );
         }
      }
      explicit vector( std::input_iterator auto first, std::input_iterator auto last,
         allocator_* p_allocator ) requires std::copyable<value_type> :
         p_allocator( p_allocator ),
         current_capacity( last - first ),
         current_size( current_capacity )
      {
         assert( p_allocator != nullptr && "Allocator cannot be nullptr" );

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         for ( size_type i = 0; first != last; ++i, ++first )
         {
            p_alloc[i] = *first;
         }
      }
      vector( std::initializer_list<type_> init, allocator_* p_allocator ) requires std::copyable<value_type> :
         p_allocator( p_allocator )
      {
         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         current_capacity = init.size( );
         current_size = init.size( );

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         for ( size_type index = 0; auto& it : init )
         {
            p_alloc[index++] = it;
         }
      }
      vector( vector const& other ) requires std::copyable<value_type>
      {
         p_allocator = other.p_allocator;
         current_size = other.current_size;
         current_capacity = other.current_capacity;

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         for ( size_type i = 0; i < current_size; ++i )
         {
            p_alloc[i] = other[i];
         }
      }
      vector( vector const& other, allocator_* p_allocator ) requires std::copyable<value_type> :
         p_allocator( p_allocator )
      {
         if ( !p_allocator )
         {
            p_allocator = other.p_allocator;
         }

         current_size = other.size( );
         current_capacity = other.size( );

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         std::copy( other.cbegin( ), other.cend( ), p_alloc );
      }
      vector( vector&& other ) noexcept
      {
         p_allocator = other.p_allocator;
         other.p_allocator = nullptr;

         current_size = other.current_size;
         other.current_size = 0;

         current_capacity = other.current_capacity;
         other.current_capacity = 0;

         p_alloc = other.p_alloc;
         other.p_alloc = nullptr;
      }
      ~vector( )
      {
         if ( p_allocator && p_alloc )
         {
            if constexpr ( basic_type<value_type> )
            {
               for ( size_type i = 0; i < current_size; ++i )
               {
                  p_alloc[i].~type_( );
               }
            }

            p_allocator->free( TO_BYTE_PTR( p_alloc ) );
            p_alloc = nullptr;
         }

         p_allocator = nullptr;
      }

      vector& operator=( vector const& other ) requires std::copyable<value_type>
      {
         if ( this != &other )
         {
            p_allocator = other.p_allocator;
            current_size = other.current_size;
            current_capacity = other.current_capacity;

            auto const size_in_bytes = current_capacity * sizeof( value_type );
            p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
            if ( !p_alloc )
            {
               throw std::bad_alloc( );
            }

            for ( size_type i = 0; i < current_size; ++i )
            {
               p_alloc[i] = other[i];
            }
         }

         return *this;
      }
      vector& operator=( vector&& other ) noexcept
      {
         if ( this != &other )
         {
            p_allocator = other.p_allocator;
            other.p_allocator = nullptr;

            current_size = other.current_size;
            other.current_size = 0;

            current_capacity = other.current_capacity;
            other.current_capacity = 0;

            p_alloc = other.p_alloc;
            other.p_alloc = nullptr;
         }

         return *this;
      }
      vector& operator=( std::initializer_list<value_type> init ) requires std::copyable<value_type>
      {
         current_capacity = p_allocator->allocation_capacity( TO_BYTE_PTR( p_alloc ) ) / sizeof( value_type );
         if ( current_capacity < init.size( ) || !p_alloc )
         {
            if ( !p_allocator->can_allocate( sizeof( value_type ) * init.size( ), alignof( value_type ) ) )
            {
               throw std::bad_alloc{ };
            }
         }

         if constexpr ( basic_type<value_type> )
         {
            std::for_each( begin( ), end( ), []( value_type& type ) {
               type.~value_type( );
            } );
         }

         if ( current_capacity < init.size( ) || !p_alloc )
         {
            p_alloc =
               TO_TYPE_PTR( p_allocator->allocate( sizeof( value_type ) * init.size( ), alignof( value_type ) ) );
         }

         current_size = init.size( );
         current_capacity = init.size( );

         for ( size_type index = 0; auto& it : init )
         {
            p_alloc[index++] = it;
         }

         return *this;
      }

      template <allocator other_ = allocator_>
      constexpr bool operator==( vector<type_, other_> const& rhs ) const requires std::equality_comparable<type_>
      {
         return std::equal( cbegin( ), cbegin( ), rhs.cbegin( ), rhs.cend( ) );
      }

      template <allocator other_ = allocator_>
      constexpr auto operator<=>( vector<type_, other_> const& rhs )
      {
         return std::lexicographical_compare_three_way(
            cbegin( ), cend( ), rhs.cbegin( ), rhs.cend( ), synth_three_way );
      }

      void assign( size_type count, value_type const& value ) requires std::copyable<value_type>
      {
         reallocate( count );

         current_size = count;
         for ( size_type i = 0; i < current_size; ++i )
         {
            p_alloc[i] = value;
         }
      }
      void assign( std::input_iterator auto first, std::input_iterator auto last ) requires std::copyable<value_type>
      {
         size_type const count = std::distance( first, last );

         reallocate( count );

         current_size = count;
         for ( size_type i = 0; first != last; ++i, ++first )
         {
            p_alloc[i] = *first;
         }
      }
      void assign( std::initializer_list<value_type> init ) requires std::copyable<value_type>
      {
         reallocate( init.size( ) );

         current_size = init.size( );
         for ( size_type i = 0; auto& it : init )
         {
            p_alloc[i++] = it;
         }
      }

      allocator_* get_allocator( ) noexcept { return p_allocator; }
      allocator_ const* get_allocator( ) const noexcept { return p_allocator; }

      // element access
      reference at( size_type index )
      {
         if ( index < 0 && index >= current_size )
         {
            throw std::out_of_range{ "Index: " + std::to_string( index ) + " is out of bounds" };
         }
         else
         {
            return p_alloc[index];
         }
      }
      const_reference at( size_type index ) const
      {
         if ( index < 0 && index >= current_size )
         {
            throw std::out_of_range{ "Index: " + std::to_string( index ) + " is out of bounds" };
         }
         else
         {
            return p_alloc[index];
         }
      }
      reference operator[]( size_type index ) noexcept
      {
         assert( index >= 0 && "Index cannot be less than zero" );
         assert( index < current_size && "Index cannot be more than vector size" );

         return p_alloc[index];
      }
      const_reference operator[]( size_type index ) const noexcept
      {
         assert( index >= 0 && "Index cannot be less than zero" );
         assert( index < current_size && "Index cannot be more than vector size" );

         return p_alloc[index];
      }
      reference front( ) noexcept { return p_alloc[0]; }
      const_reference front( ) const noexcept { return p_alloc[0]; }

      reference back( ) noexcept { return p_alloc[current_size - 1]; }
      const_reference back( ) const noexcept { return p_alloc[current_size - 1]; }

      // data access
      pointer data( ) noexcept { return p_alloc; }
      const_pointer data( ) const noexcept { return p_alloc; }

      // iterators
      constexpr iterator begin( ) noexcept { return iterator{ p_alloc }; }
      constexpr const_iterator cbegin( ) const noexcept { return const_iterator{ p_alloc }; }

      constexpr iterator end( ) noexcept { return iterator{ p_alloc + current_size }; }
      constexpr const_iterator cend( ) const noexcept { return const_iterator{ p_alloc + current_size }; }

      constexpr reverse_iterator rbegin( ) noexcept { return reverse_iterator{ p_alloc }; }
      constexpr const_reverse_iterator crbegin( ) const noexcept { return const_reverse_iterator{ p_alloc }; }

      constexpr reverse_iterator rend( ) noexcept { return reverse_iterator{ p_alloc + current_size }; }
      constexpr const_reverse_iterator crend( ) const noexcept
      {
         return const_reverse_iterator{ p_alloc + current_size };
      }

      // capacity
      constexpr bool empty( ) const noexcept { return current_size == 0; }
      constexpr size_type size( ) const noexcept { return current_size; }
      constexpr size_type max_size( ) const noexcept { return std::numeric_limits<std::uintptr_t>::max( ); }
      void reserve( size_type new_capacity )
      {
         if ( new_capacity > max_size( ) )
         {
            throw std::length_error{ "number of elements " + std::to_string( new_capacity ) + " is too big" };
         }

         if ( new_capacity > current_capacity )
         {
            auto* p_temp = reinterpret_cast<pointer>( p_allocator->allocate( new_capacity, alignof( value_type ) ) );
            if ( !p_temp )
            {
               throw std::bad_alloc{ };
            }

            for ( size_type i = 0; i < current_size; ++i )
            {
               p_temp[i] = std::move( p_alloc[i] );
            }

            p_allocator->free( TO_BYTE_PTR( p_alloc ) );
            current_capacity = new_capacity;
         }
      }
      constexpr size_type capacity( ) const noexcept { return current_capacity; };

      // modifiers
      void clear( ) noexcept
      {
         if constexpr ( !basic_type<value_type> )
         {
            for ( size_type i = 0; i < current_size; ++i )
            {
               p_alloc[i].~type_( );
            }
         }

         current_size = 0;
      }
      iterator insert( const_iterator pos, const value_type& value ) requires std::copyable<value_type>
      {
         size_type const new_size = current_size + 1;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size++ == 0 )
         {
            p_alloc[0] = value;

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[( &( *pos ) - p_alloc )] };
            for ( auto last = end( ) - 1; last != beg; --last )
            {
               std::iter_swap( last, last - 1 );
            }

            *beg = value;

            return beg;
         }
      }
      iterator insert( const_iterator pos, value_type&& value ) requires std::movable<value_type>
      {
         size_type const new_size = current_size + 1;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size++ == 0 )
         {
            p_alloc[0] = std::move( value );

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[( &( *pos ) - p_alloc )] };
            for ( auto last = end( ) - 1; last != beg; --last )
            {
               std::iter_swap( last, last - 1 );
            }

            *beg = std::move( value );

            return beg;
         }
      }
      iterator insert( const_iterator pos, size_type count, const value_type& value ) requires std::copyable<value_type>
      {
         size_type const new_size = current_size + count;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size == 0 )
         {
            for ( size_type i = 0; i < new_size; ++i )
            {
               p_alloc[i] = value;
            }

            current_size = new_size;

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[&( *pos ) - p_alloc] };

            current_size = new_size;

            for ( auto last = end( ) - count - 1; last >= beg; --last )
            {
               std::iter_swap( last, last + count );
            }

            for ( auto last = beg + count - 1; last >= beg; --last )
            {
               *last = value;
            }

            return beg;
         }
      }
      iterator insert( const_iterator pos, std::input_iterator auto first,
         std::input_iterator auto last ) requires std::copyable<value_type>
      {
         size_type const count = last - first;
         size_type const new_size = current_size + count;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size == 0 )
         {
            for ( size_type i = 0; first != last; ++first, ++i )
            {
               p_alloc[i] = *first;
            }

            current_size = new_size;

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[&( *pos ) - p_alloc] };

            current_size = new_size;

            for ( auto last = end( ) - count - 1; last >= beg; --last )
            {
               std::iter_swap( last, last + count );
            }

            for ( auto last = beg + count - 1; last >= beg; --last, ++first )
            {
               *last = *first;
            }

            return beg;
         }
      }
      iterator insert( const_iterator pos, std::initializer_list<value_type> init ) requires std::copyable<value_type>
      {
         size_type const new_size = current_size + init.size( );

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size == 0 )
         {
            current_size = new_size;

            for ( size_type i = 0; auto& it : init )
            {
               p_alloc[i++] = it;
            }

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[&( *pos ) - p_alloc] };

            current_size = new_size;

            for ( auto last = end( ) - init.size( ) - 1; last >= beg; --last )
            {
               std::iter_swap( last, last + init.size( ) );
            }

            auto last = beg + init.size( ) - 1;
            auto init_l = init.end( ) - 1;
            for ( ; last >= beg; --last, --init_l )
            {
               *last = *init_l;
            }

            return beg;
         }
      }
      template <class... args_>
      iterator emplace( const_iterator pos, args_&&... args ) requires std::constructible_from<value_type, args_...>
      {
         size_type const new_size = current_size + 1;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size++ == 0 )
         {
            new ( TO_BYTE_PTR( p_alloc ) ) value_type( args... );

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[( &( *pos ) - p_alloc )] };
            for ( auto last = end( ) - 1; last != beg; --last )
            {
               std::iter_swap( last, last - 1 );
            }

            new ( TO_TYPE_PTR( &( *beg ) ) ) value_type( args... );

            return beg;
         }
      }

      iterator erase( const_iterator pos ) noexcept
      {
         if ( pos == cend( ) )
         {
            return end( );
         }
         else
         {
            auto it = iterator{ &p_alloc[( &( *pos ) - p_alloc )] };
            for ( auto curr = it; curr != end( ) - 1; ++curr )
            {
               std::iter_swap( curr, curr + 1 );
            }

            if constexpr ( basic_type<value_type> )
            {
               ( end( ) - 1 )->~value_type( );
            }

            --current_size;

            return it;
         }
      }
      iterator erase( const_iterator first, const_iterator last ) noexcept
      {
         size_type const count = std::distance( first, last );

         auto it = iterator{ &p_alloc[( &( *first ) - p_alloc )] };
         for ( auto curr = it; curr + count != end( ); ++curr )
         {
            std::iter_swap( curr, curr + count );
         }

         if constexpr ( !basic_type<value_type> )
         {
            for ( auto beg = end( ) - count; beg != end( ); ++beg )
            {
               beg->~value_type( );
            }
         }

         current_size = current_size - count;

         return it;
      }

      void push_back( const_reference value ) requires std::copyable<value_type>
      {
         assert( current_size + 1 < max_size( ) );

         reallocate( current_size + 1 );

         p_alloc[current_size++] = value;
      }
      void push_back( value_type&& value ) requires std::movable<value_type>
      {
         assert( current_size + 1 < max_size( ) );

         reallocate( current_size + 1 );

         p_alloc[current_size++] = std::move( value );
      }

      template <class... args_>
      reference emplace_back( args_&&... args ) requires std::constructible_from<value_type, args_...>
      {
         assert( current_size + 1 < max_size( ) );

         reallocate( current_size + 1 );

         new ( TO_BYTE_PTR( p_alloc + current_size ) ) value_type( args... );

         return p_alloc[current_size++];
      }

      void pop_back( )
      {
         if constexpr ( !basic_type<value_type> )
         {
            ( end( ) - 1 )->~value_type( );
         }

         --current_size;
      }

      void resize( size_type count ) requires std::default_initializable<value_type>
      {
         assert( count != 0 );
         assert( count <= max_size( ) );

         if ( count < current_size )
         {
            if constexpr ( !basic_type<value_type> )
            {
               for ( int i = count; i < current_size; ++i )
               {
                  p_alloc[i].~value_type( );
               }
            }

            current_size = count;
         }
         else if ( count > current_size )
         {
            reallocate( count );

            for ( int i = current_size; i < count; ++i )
            {
               new ( TO_BYTE_PTR( p_alloc + i ) ) value_type( );
            }

            current_size = count;
         }
      }
      void resize( size_type count, const_reference value ) requires std::copyable<value_type>
      {
         assert( count != 0 );
         assert( count <= max_size( ) );

         if ( count < current_size )
         {
            if constexpr ( !basic_type<value_type> )
            {
               for ( int i = count; i < current_size; ++i )
               {
                  p_alloc[i].~value_type( );
               }
            }

            current_size = count;
         }
         else if ( count > current_size )
         {
            reallocate( count );

            for ( int i = current_size; i < count; ++i )
            {
               new ( TO_BYTE_PTR( p_alloc + i ) ) value_type( value );
            }

            current_size = count;
         }
      }

   private:
      void reallocate( size_type new_size )
      {
         if ( !p_alloc )
         {
            if ( !p_allocator->can_allocate( sizeof( value_type ) * new_size, alignof( value_type ) ) )
            {
               throw std::bad_alloc{ };
            }
            else
            {
               p_alloc = TO_TYPE_PTR( p_allocator->allocate( sizeof( value_type ) * new_size, alignof( value_type ) ) );
               current_capacity = new_size;
            }
         }
         else if ( new_size > current_capacity )
         {
            current_capacity = p_allocator->allocation_capacity( TO_BYTE_PTR( p_alloc ) ) / sizeof( value_type );

            if ( new_size > current_capacity )
            {

               if ( !p_allocator->can_allocate( sizeof( value_type ) * new_size, alignof( value_type ) ) )
               {
                  throw std::bad_alloc{ };
               }
               else
               {
                  pointer p_temp =
                     TO_TYPE_PTR( p_allocator->allocate( sizeof( value_type ) * new_size, alignof( value_type ) ) );

                  for ( size_type i = 0; i < current_size; ++i )
                  {
                     if constexpr ( std::movable<value_type> )
                     {
                        p_temp[i] = std::move( p_alloc[i] );
                     }
                     else
                     {
                        p_temp[i] = p_alloc[i];
                     }

                     if constexpr ( basic_type<value_type> )
                     {
                        p_alloc[i].~value_type( );
                     }
                  }

                  p_alloc = p_temp;
                  current_capacity = new_size;
               }
            }
         }
      }

   private:
      allocator_* p_allocator{ nullptr };

      pointer p_alloc{ nullptr };
      size_type current_capacity{ 0 };
      size_type current_size{ 0 };
   }; // namespace ESL
} // namespace ESL

#undef TO_TYPE_PTR
