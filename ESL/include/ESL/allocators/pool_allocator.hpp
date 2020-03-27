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
#include <cstdint>
#include <functional>
#include <memory>

namespace ESL
{
   class pool_allocator final
   {
      struct block_header
      {
         block_header* p_next;
      };

   public:
      using pointer = std::byte*;
      using size_type = std::size_t;

   public:
      pool_allocator( size_type const block_count, size_type const block_size ) noexcept;

      [[nodiscard]] pointer allocate( size_type size, size_type alignment ) noexcept;
      void free( pointer p_location ) noexcept;

      [[nodiscard]] bool can_allocate( size_type size, size_type alignment ) const noexcept;

      size_type allocation_capacity( pointer alloc ) const noexcept;

      size_type max_size( ) const noexcept;
      size_type memory_usage( ) const noexcept;
      size_type allocation_count( ) const noexcept;

      template <class type_, class... args_>
      [[nodiscard]] auto_ptr<type_> make_unique( args_&&... args ) noexcept
      {
         return auto_ptr<type_>( make_new<type_>( args... ), [this]( type_* p_type ) {
            this->make_delete( p_type );
         } );
      }

      template <class type_, class... args_>
      [[nodiscard]] type_* make_new( args_&&... args ) noexcept requires std::constructible_from<type_, args_...>
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
      void make_delete( type_* p_type ) noexcept
      {
         if ( p_type )
         {
            p_type->~type_( );
            free( TO_BYTE_PTR( p_type ) );
         }
      }

   private:
      size_type total_size;
      size_type used_memory;
      size_type num_allocations;

      size_type block_count = 0;
      size_type block_size = 0;

      std::unique_ptr<std::byte[]> p_memory;
      block_header* p_first_free = nullptr;
   };
} // namespace ESL
