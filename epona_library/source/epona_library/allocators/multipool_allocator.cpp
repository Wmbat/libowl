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

#include <epona_library/allocators/multipool_allocator.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

#define TO_ACCESS_HEADER_PTR( ptr ) reinterpret_cast<access_header*>( ptr )
#define TO_BLOCK_HEADER_PTR( ptr ) reinterpret_cast<block_header*>( ptr )

namespace ESL
{
   multipool_allocator::multipool_allocator( create_info const& create_info ) noexcept :
      pool_count( create_info.pool_count ), pool_size( create_info.pool_size ), depth( create_info.depth ),
      total_size( pool_count * pool_size * depth )
   {
      assert( pool_count != 0 && "Cannot have no blocks in memory pool" );
      assert( pool_size != 0 && "Cannot have a block size of zero" );
      assert( depth != 0 && "Cannot have a pool depth of zero" );

      size_type true_alloc_size = 0;
      for ( size_type i = 0; i < depth; ++i )
      {
         size_type const depth_pow = std::pow( 2, i );
         size_type const depth_pool_count = pool_count * depth_pow;
         size_type const depth_pool_size = pool_size / depth_pow;

         true_alloc_size += depth_pool_count * ( sizeof( block_header ) + depth_pool_size );
      }

      p_memory = std::make_unique<std::byte[]>( true_alloc_size + ( ( depth + 1 ) * sizeof( access_header ) ) );

      p_access_headers = TO_ACCESS_HEADER_PTR( p_memory.get( ) );
      for ( size_type i = 0; i < depth; ++i )
      {
         size_type const depth_pow = std::pow( 2, i );
         size_type const depth_offset = ( depth + 1 ) * sizeof( block_header );
         size_type const pool_offset = pool_count * pool_size * i;

         p_access_headers[i].p_first_free = TO_BLOCK_HEADER_PTR( p_memory.get( ) + depth_offset + pool_offset );

         block_header* p_first_free = p_access_headers[i].p_first_free;
         p_first_free->depth_index = i;

         for ( size_type j = 1; j < pool_count * depth_pow; ++j )
         {
            size_type const offset = j * ( pool_size / depth_pow + sizeof( block_header ) );

            auto* p_new = TO_BLOCK_HEADER_PTR( p_memory.get( ) + offset + depth_offset + pool_offset );
            p_new->depth_index = i;
            p_first_free->p_next = p_new;
            p_first_free = p_new;
            p_first_free->p_next = nullptr;
         }
      }
   }

   auto multipool_allocator::allocate( size_type size, size_type alignment ) noexcept -> pointer
   {
      assert( size != 0 && "Allocation size cannot be zero" );
      assert( size <= pool_size && "Allocation size cannot be greater than max pool size" );
      assert( alignment != 0 && "Allocation alignment cannot be zero" );

      size_type depth_index = 0;
      for ( size_type i = 0; i < depth; ++i )
      {
         if ( pool_size / std::pow( 2, i ) >= size )
         {
            depth_index = i;
         }
      }

      if ( p_access_headers[depth_index].p_first_free )
      {
         block_header* p_block_header = p_access_headers[depth_index].p_first_free;
         p_block_header->depth_index = depth_index;
         p_access_headers[depth_index].p_first_free = p_block_header->p_next;

         used_memory += pool_size;
         ++num_allocations;

         return static_cast<void*>( ++p_block_header );
      }
      else
      {
         return nullptr;
      }
   }

   auto multipool_allocator::reallocate( pointer p_alloc, size_type new_size ) noexcept -> pointer
   {
      new_size = 0;

      return p_alloc;
   }

   auto multipool_allocator::deallocate( pointer p_alloc ) noexcept -> void
   {
      assert( p_alloc != nullptr && "cannot free a nullptr" );

      auto* p_header = reinterpret_cast<block_header*>( static_cast<std::byte*>( p_alloc ) - sizeof( block_header ) );
      p_header->p_next = p_access_headers[p_header->depth_index].p_first_free;
      p_access_headers[p_header->depth_index].p_first_free = p_header;

      used_memory -= pool_size;
      --num_allocations;
   }

   auto multipool_allocator::allocation_capacity( pointer p_alloc ) const noexcept -> size_type
   {
      assert( p_alloc != nullptr && "Cannot access nullptr" );

      auto* p_header = TO_BLOCK_HEADER_PTR( p_alloc );
      --p_header;

      return pool_size / std::pow( 2, p_header->depth_index );
   }

   auto multipool_allocator::release( ) noexcept -> void
   {
      p_access_headers = reinterpret_cast<access_header*>( p_memory.get( ) );
      for ( size_type i = 0; i < depth; ++i )
      {
         size_type const depth_pow = std::pow( 2, i );
         size_type const depth_offset = ( depth + 1 ) * sizeof( block_header );
         size_type const pool_offset = pool_count * pool_size * i;

         p_access_headers[i].p_first_free = TO_BLOCK_HEADER_PTR( p_memory.get( ) + depth_offset + pool_offset );

         auto* p_base_cpy = p_access_headers[i].p_first_free;
         for ( size_type j = 1; j < pool_count * depth_pow; ++j )
         {
            size_type const offset = j * ( pool_size / depth_pow + sizeof( block_header ) );

            auto* p_new = TO_BLOCK_HEADER_PTR( p_memory.get( ) + offset + depth_offset + pool_offset );
            p_base_cpy->p_next = p_new;
            p_base_cpy = p_new;
            p_base_cpy->p_next = nullptr;
         }
      }

      used_memory = 0;
      num_allocations = 0;
   }

   auto multipool_allocator::max_size( ) const noexcept -> size_type { return total_size; }
   auto multipool_allocator::memory_usage( ) const noexcept -> size_type { return used_memory; }
   auto multipool_allocator::allocation_count( ) const noexcept -> size_type { return num_allocations; }
} // namespace ESL

#undef TO_BLOCK_HEADER_PTR
#undef TO_ACCESS_HEADER_PTR
