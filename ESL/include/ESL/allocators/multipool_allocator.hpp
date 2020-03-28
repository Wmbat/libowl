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

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

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

#include <cassert>
#include <memory>

namespace ESL
{
   class multipool_allocator final
   {
   public:
      using pointer = std::byte*;
      using size_type = std::size_t;

      struct block_header
      {
         block_header* p_next = nullptr;
         size_type depth_index = 0;
      };

      struct access_header
      {
         block_header* p_first_free = nullptr;
      };

   public:
      multipool_allocator( size_type block_count, size_type block_size, size_type pool_depth = 1 ) noexcept;

      [[nodiscard( "Allocation will never be freed" )]] pointer allocate(
         size_type size, size_type alignment ) noexcept;
      void free( pointer p_alloc ) noexcept;

      [[nodiscard]] bool can_allocate( size_type size, size_type alignment ) const noexcept;

      size_type allocation_capacity( pointer alloc ) const noexcept;

      void clear( ) noexcept;

      size_type max_size( ) const noexcept;
      size_type memory_usage( ) const noexcept;
      size_type allocation_count( ) const noexcept;

      template <class type_, class... args_>
      [[nodiscard]] type_* make_new( args_&&... args ) noexcept
      {
         if ( auto* p_alloc = allocate( sizeof( type_ ), alignof( type_ ) ) )
         {
            return new ( p_alloc ) type_( args... );
         }
         else
         {
            return nullptr;
         }
      }

      template <class type_>
      [[nodiscard]] type_* make_array( size_type element_count ) noexcept
      {
         assert( element_count != 0 && "cannot allocate zero elements" );
         static_assert( std::is_default_constructible_v<type_>, "type must be default constructible" );

         auto* p_alloc = allocate( sizeof( type_ ) * element_count, alignof( type_ ) );
         if ( !p_alloc )
         {
            return nullptr;
         }

         for ( std::size_t i = 0; i < element_count; ++i )
         {
            new ( p_alloc + ( sizeof( type_ ) * i ) ) type_( );
         }

         return reinterpret_cast<type_*>( p_alloc );
      }

      template <class type_>
      void make_delete( type_* p_type ) noexcept
      {
         if ( p_type )
         {
            p_type->~type_( );
            free( TO_BYTE_PTR( p_type ) );
         }
      }

      template <class type_>
      void make_delete( type_* p_type, size_type element_count ) noexcept
      {
         assert( element_count != 0 && "cannot free zero elements" );

         for ( size_type i = 0; i < element_count; ++i )
         {
            p_type[i].~type_( );
         }

         free( TO_BYTE_PTR( p_type ) );
      }

      template <class type_, class... args_>
      [[nodiscard]] auto_ptr<type_> make_unique( args_&&... args ) noexcept
      {
         return auto_ptr<type_>( make_new<type_>( args... ), [this]( type_* p_type ) {
            this->make_delete( p_type );
         } );
      }

   private:
      size_type block_count;
      size_type block_size;
      size_type pool_depth;

      size_type total_size;
      size_type used_memory;
      size_type num_allocations;

      std::unique_ptr<std::byte[]> p_memory;
      access_header* p_access_headers;
   };
} // namespace ESL
