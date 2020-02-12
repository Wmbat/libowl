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

#include <EML/allocator_utils.hpp>

#include <memory>

namespace EML
{
   class multipool_allocator final
   {
   public:
      struct block_header
      {
         block_header* p_next = nullptr;
      };

      template <class type_>
      struct pointer
      {
         type_* p_data = nullptr;
         std::size_t const index = 0;
      };

   public:
      multipool_allocator( std::size_t block_count, std::size_t block_size, std::size_t pool_depth = 1 ) noexcept;

      pointer<std::byte> allocate( std::size_t size, std::size_t alignment ) noexcept;
      void free( pointer<std::byte> alloc ) noexcept;

      template <class type_, class... args_>
      [[nodiscard]] pointer<type_> make_new( args_&&... args ) noexcept
      {
         auto const alloc = allocate( sizeof( type_ ), alignof( type_ ) );
         if ( alloc.p_data )
         {
            return {new ( alloc.p_data ) type_( args... ), alloc.index};
         }
         else
         {
            return {nullptr, 0};
         }
      }

      template <class type_>
      void make_delete( pointer<type_> type ) noexcept
      {
         if ( type.p_data )
         {
            type.p_data->~type_( );
            free( type );
         }
      }

      void clear( ) noexcept;

      std::size_t max_size( ) const noexcept;
      std::size_t memory_usage( ) const noexcept;
      std::size_t allocation_count( ) const noexcept;

   private:
      std::size_t block_count;
      std::size_t block_size;
      std::size_t pool_depth;

      std::size_t total_size;
      std::size_t used_memory;
      std::size_t num_allocations;

      std::unique_ptr<std::byte[]> p_memory;
      block_header* p_depth_header;
   };
} // namespace EML
