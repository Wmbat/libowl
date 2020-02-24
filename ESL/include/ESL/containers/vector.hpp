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
#include <ESL/utils/input_iterator.hpp>
#include <ESL/utils/random_access_iterator.hpp>

#include <cassert>
#include <iterator>
#include <memory>
#include <type_traits>

namespace ESL
{
   template <class type_, class allocator_>
   class vector
   {
      static_assert( std::is_base_of_v<allocation_interface, allocator_>,
         "allocator type must be a child of allocation_interface" );
      static_assert( enable_reallocation<allocator_>::enable == true, "allocator type does not support reallocation" );

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
      explicit vector( size_type count, const_reference value, allocator_* p_allocator ) :
         p_allocator( p_allocator ), current_capacity( count ), current_size( count )
      {
         static_assert( std::is_copy_constructible_v<type_>, "value type is not copy constructible" );
         static_assert( std::is_copy_assignable_v<type_>, "value type is not copy assignable" );

         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         p_alloc = reinterpret_cast<pointer>( p_allocator->allocate( current_capacity, alignof( value_type ) ) );

         if ( !p_alloc )
         {
            throw std::bad_alloc{};
         }

         for ( size_type i = 0; i < current_size; ++i )
         {
            new ( reinterpret_cast<std::byte*>( p_alloc + i ) ) value_type( value );
         }
      }
      explicit vector( size_type count, allocator_* p_allocator ) noexcept :
         p_allocator( p_allocator ), current_capacity( count ), current_size( count )
      {
         static_assert( std::is_default_constructible_v<value_type>, "value type must be default constructible" );

         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         p_alloc = reinterpret_cast<pointer>( p_allocator->allocate( current_capacity, alignof( value_type ) ) );

         if ( !p_alloc )
         {
            throw std::bad_alloc{};
         }

         for ( size_type i = 0; i < current_size; ++i )
         {
            new ( reinterpret_cast<std::byte*>( p_alloc + i ) ) value_type( );
         }
      }
      template <class input_it_>
      vector( input_it_ first, input_it_ last, allocator_* p_allocator ) :
         p_allocator( p_allocator ), current_capacity( last - first ), current_size( current_capacity )
      {
         auto test = last - first;

         static_assert( std::is_copy_constructible_v<value_type>, "value type is not copy constructible" );
         static_assert( std::is_copy_assignable_v<value_type>, "value type is not copy assignable" );

         p_alloc = reinterpret_cast<pointer>( p_allocator->allocate( current_capacity, alignof( value_type ) ) );

         if ( !p_alloc )
         {
            throw std::bad_alloc{};
         }

         for ( size_type i = 0; first < last; ++i, ++first )
         {
            p_alloc[i] = *first;
         }
      }
      vector( vector const& other )
      {
         static_assert( std::is_copy_constructible_v<type_>, "value type is not copy constructible" );
         static_assert( std::is_copy_assignable_v<type_>, "value type is not copy assignable" );

         p_allocator = other.p_allocator;
         current_size = other.current_size;
         current_capacity = other.current_capacity;

         p_alloc = reinterpret_cast<pointer>( p_allocator->allocate( current_capacity, alignof( value_type ) ) );

         if ( !p_alloc )
         {
            throw std::bad_alloc{};
         }

         for ( size_type i = 0; i < current_size; ++i )
         {
            p_alloc[i] = other[i];
         }
      }
      vector( vector const& other, allocator_* p_allocator ) : p_allocator( p_allocator )
      {
         static_assert( std::is_copy_constructible_v<value_type>, "value type is copy constructible" );
         static_assert( std::is_copy_assignable_v<value_type>, "value type is not copy assignable" );

         if ( !p_allocator )
         {
            p_allocator = other.p_allocator;
         }

         p_alloc = reinterpret_cast<pointer>( p_allocator->allocate( other.capacity( ), alignof( value_type ) ) );

         if ( !p_alloc )
         {
            throw std::bad_alloc{};
         }

         current_size = other.size( );
         current_capacity = other.size( );

         for ( size_type i = 0; i < current_size; ++i )
         {
            p_alloc[i] = other[i];
         }
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
      vector( std::initializer_list<type_> init, allocator_* p_allocator ) : p_allocator( p_allocator )
      {
         static_assert( std::is_copy_constructible_v<value_type>, "value type is not copy constructible" );
         static_assert( std::is_copy_assignable_v<value_type>, "value type is not copy assignable" );

         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         current_capacity = init.size( );
         current_size = init.size( );

         p_alloc = reinterpret_cast<pointer>( p_allocator->allocate( current_capacity, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc{};
         }

         for ( size_type index = 0; auto& it : init )
         {
            p_alloc[index++] = it;
         }
      }
      ~vector( )
      {
         if ( p_allocator && p_alloc )
         {
            p_allocator->free( reinterpret_cast<std::byte*>( p_alloc ) );
            p_alloc = nullptr;
         }

         p_allocator = nullptr;
      }

      vector& operator=( vector const& other )
      {
         static_assert( std::is_copy_constructible_v<value_type>, "value type is not copy constructible" );
         static_assert( std::is_copy_assignable_v<value_type>, "value type is not copy assignable" );

         if ( this != &other )
         {
            p_allocator = other.p_allocator;
            current_size = other.current_size;
            current_capacity = other.current_capacity;

            p_alloc = p_allocator->allocate( current_capacity, alignof( type_ ) );

            if ( !p_alloc )
            {
               throw std::bad_alloc{};
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

      template <class input_it_>
      void assign( input_it_ first, input_it_ last )
      {
         static_assert( std::is_copy_constructible_v<value_type>, "value type is not copy constructible" );
         static_assert( std::is_copy_assignable_v<value_type>, "value type is not copy assignable" );

         size_type elem_count = std::distance( first, last );

         if ( elem_count <= current_capacity )
         {
            for ( size_type i = 0; first != last; ++first, ++i )
            {
               p_alloc[i] = first;
            }

            current_size = elem_count;
         }
         else
         {
            auto* p_temp = reinterpret_cast<pointer>( p_allocator->allocate( elem_count, alignof( value_type ) ) );
            if ( !p_temp )
            {
               throw std::bad_alloc{};
            }

            for ( size_type i = 0; first != last; ++first, ++i )
            {
               p_temp[i] = first;
            }

            p_allocator->free( reinterpret_cast<std::byte*>( p_alloc ) );
            p_alloc = p_temp;

            current_capacity = elem_count;
            current_size = elem_count;
         }
      }
      void assign( size_type count, const_reference value )
      {
         static_assert( std::is_copy_constructible_v<value_type>, "value type is not copy constructible" );
         static_assert( std::is_copy_assignable_v<value_type>, "value type is not copy assignable" );

         if ( count > max_size( ) )
         {
            throw std::length_error{"number of elements " + std::to_string( count ) + " is too big"};
         }

         if ( count <= current_capacity )
         {
            for ( size_type i = 0; i < current_size; ++i )
            {
               p_alloc[i] = value;
            }

            current_size = count;
         }
         else
         {
            auto* p_temp = reinterpret_cast<pointer>( p_allocator->allocate( count, alignof( value_type ) ) );
            if ( !p_temp )
            {
               throw std::bad_alloc{};
            }

            for ( size_type i = 0; i < count; ++i )
            {
               p_temp[i] = value;
            }

            p_allocator->free( reinterpret_cast<std::byte*>( p_alloc ) );
            p_alloc = p_temp;

            current_capacity = count;
            current_size = count;
         }
      }
      void assign( std::initializer_list<value_type> init_list )
      {
         static_assert( std::is_copy_constructible_v<value_type>, "value type is not copy constructible" );
         static_assert( std::is_copy_assignable_v<value_type>, "value type is not copy assignable" );

         size_type count = init_list.size( );
         if ( count > max_size( ) )
         {
            throw std::length_error{"number of elements " + std::to_string( count ) + " is too big"};
         }

         if ( count <= current_capacity )
         {
            for ( size_type i = 0; auto it : init_list )
            {
               p_alloc[i++] = it;
            }

            current_size = count;
         }
         else
         {
            auto* p_temp = reinterpret_cast<pointer>( p_allocator->allocate( count, alignof( value_type ) ) );
            if ( !p_temp )
            {
               throw std::bad_alloc{};
            }

            for ( size_type i = 0; auto it : init_list )
            {
               p_alloc[i++] = it;
            }

            p_allocator->free( reinterpret_cast<std::byte*>( p_alloc ) );
            p_alloc = p_temp;

            current_capacity = count;
            current_size = count;
         }
      }

      allocator_* get_allocator( ) noexcept { return p_allocator; }

      reference at( size_type index )
      {
         if ( index < 0 && index >= current_size )
         {
            throw std::out_of_range( "Index array " + std::to_string( index ) + "out of bounds" );
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
            throw std::out_of_range( "Index array " + std::to_string( index ) + "out of bounds" );
         }
         else
         {
            return p_alloc[index];
         }
      }

      reference operator[]( size_type index )
      {
         assert( index >= 0 && "Index cannot be less than zero" );
         assert( index < current_size && "Index cannot be more than array size" );

         return p_alloc[index];
      }
      const_reference operator[]( size_type index ) const
      {
         assert( index >= 0 && "Index cannot be less than zero" );
         assert( index < current_size && "Index cannot be more than array size" );

         return p_alloc[index];
      }

      // element access
      reference front( ) { return p_alloc[0]; }
      const_reference front( ) const { return p_alloc[0]; }

      reference back( ) { return p_alloc[current_size - 1]; }
      const_reference back( ) const { return p_alloc[current_size - 1]; }

      pointer data( ) { return p_alloc; }
      const_pointer data( ) const { return p_alloc; }

      // iterators
      constexpr iterator begin( ) noexcept { return iterator{p_alloc}; }
      constexpr const_iterator cbegin( ) const noexcept { return const_iterator{p_alloc}; }

      constexpr iterator end( ) noexcept { return iterator{p_alloc + current_size}; }
      constexpr const_iterator cend( ) const noexcept { return const_iterator{p_alloc + current_size}; }

      constexpr reverse_iterator rbegin( ) noexcept { return reverse_iterator{p_alloc}; }
      constexpr const_reverse_iterator crbegin( ) const noexcept { return const_reverse_iterator{p_alloc}; }

      constexpr reverse_iterator rend( ) noexcept { return reverse_iterator{p_alloc + current_size}; }
      constexpr const_reverse_iterator crend( ) const noexcept
      {
         return const_reverse_iterator{p_alloc + current_size};
      }

      // capacity
      constexpr bool empty( ) const noexcept { return current_size == 0; }
      constexpr size_type size( ) const noexcept { return current_size; }
      constexpr size_type max_size( ) const noexcept { return std::numeric_limits<std::uintptr_t>::max( ); }
      void reserve( size_type new_capacity )
      {
         if ( new_capacity > max_size( ) )
         {
            throw std::length_error{"number of elements " + std::to_string( new_capacity ) + " is too big"};
         }

         if ( new_capacity > current_capacity )
         {
            auto* p_temp = reinterpret_cast<pointer>( p_allocator->allocate( new_capacity, alignof( value_type ) ) );
            if ( !p_temp )
            {
               throw std::bad_alloc{};
            }

            for ( size_type i = 0; i < current_size; ++i )
            {
               p_temp[i] = std::move( p_alloc[i] );
            }

            p_allocator->free( reinterpret_cast<std::byte*>( p_alloc ) );
            current_capacity = new_capacity;
         }
      }
      constexpr size_type capacity( ) const noexcept { return current_capacity; };

      void clear( ) noexcept
      {
         for ( size_type i = 0; i < current_size; ++i )
         {
            p_alloc[i].~type_( );
         }

         current_size = 0;
      }

   private:
      allocator_* p_allocator{nullptr};

      pointer p_alloc{nullptr};
      size_type current_capacity{0};
      size_type current_size{0};
   };
} // namespace ESL
