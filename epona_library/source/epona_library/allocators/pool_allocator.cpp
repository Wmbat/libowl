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

#include <epona_library/allocators/pool_allocator.hpp>

#include <cstdint>
#include <utility>

#define TO_POOL_HEADER_PTR( ptr ) reinterpret_cast<pool_header*>( ptr )

namespace ESL
{
   pool_allocator::pool_allocator( create_info const& create_info ) noexcept :
      total_size( create_info.pool_count * create_info.pool_size ), pool_count( create_info.pool_count ),
      pool_size( create_info.pool_size ),
      p_memory( std::make_unique<std::byte[]>( pool_count * ( pool_size + sizeof( pool_header ) ) ) ),
      p_first_free( nullptr )
   {
      assert( pool_count != 0 && "Cannot have no blocks in memory pool" );
      assert( pool_size != 0 && "Cannot have a block size of zero" );

      p_first_free = TO_POOL_HEADER_PTR( p_memory.get( ) );
      auto* p_base_cpy = p_first_free;

      for ( size_type i = 1; i < pool_count; ++i )
      {
         size_type offset = i * ( pool_size + sizeof( pool_header ) );

         auto* p_new = TO_POOL_HEADER_PTR( p_memory.get( ) + offset );
         p_base_cpy->p_next = p_new;
         p_base_cpy = p_new;
         p_base_cpy->p_next = nullptr;
      }
   }

   auto pool_allocator::allocate( size_type size, size_type alignment ) noexcept -> pointer
   {
      assert( size != 0 && "Allocation size cannot be zero" );
      assert( alignment != 0 && "Allocation alignment cannot be zero" );

      if ( p_first_free )
      {
         std::byte* p_pool_header = TO_BYTE_PTR( p_first_free );

         p_first_free = p_first_free->p_next;

         used_memory += pool_size;
         ++num_allocations;

         return static_cast<void*>( p_pool_header + sizeof( pool_header ) );
      }
      else
      {
         return nullptr;
      }
   }

   auto pool_allocator::reallocate( pointer p_alloc, size_type new_size ) noexcept -> pointer
   {
      assert( new_size != 0 );
      assert( p_alloc != nullptr );

      if ( new_size <= pool_size )
      {
         return p_alloc;
      }
      else
      {
         return nullptr;
      }
   }

   auto pool_allocator::deallocate( pointer p_alloc ) noexcept -> void
   {
      assert( p_alloc != nullptr && "cannot free a nullptr" );
      assert( static_cast<std::int32_t>( num_allocations ) - 1 >= 0 );

      auto* p_header = TO_POOL_HEADER_PTR( static_cast<std::byte*>( p_alloc ) - sizeof( pool_header ) );
      p_header->p_next = p_first_free;
      p_first_free = p_header;

      used_memory -= pool_size;
      --num_allocations;
   }

   auto pool_allocator::allocation_capacity( pointer p_alloc ) const noexcept -> size_type
   {
      if ( p_alloc )
      {
         return pool_size;
      }
      else
      {
         return 0;
      }
   }

   auto pool_allocator::max_size( ) const noexcept -> size_type { return total_size; }
   auto pool_allocator::memory_usage( ) const noexcept -> size_type { return used_memory; }
   auto pool_allocator::allocation_count( ) const noexcept -> size_type { return num_allocations; }
} // namespace ESL

#undef TO_POOL_HEADER_PTR
