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

#include <ESL/allocators/multipool_allocator.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

#define TO_ACCESS_HEADER_PTR( ptr ) reinterpret_cast<access_header*>( ptr )
#define TO_BLOCK_HEADER_PTR( ptr ) reinterpret_cast<block_header*>( ptr )

namespace ESL
{
   multipool_allocator::multipool_allocator(
      size_type block_count, size_type block_size, size_type pool_depth ) noexcept :
      block_count( block_count ),
      block_size( block_size ), pool_depth( pool_depth ), total_size( block_count * block_size * pool_depth ),
      used_memory( 0 ), num_allocations( 0 )
   {
      assert( block_count != 0 && "Cannot have no blocks in memory pool" );
      assert( block_size != 0 && "Cannot have a block size of zero" );
      assert( pool_depth != 0 && "Cannot have a pool depth of zero" );

      size_type true_alloc_size = 0;
      for ( int i = 0; i < pool_depth; ++i )
      {
         size_type const depth_pow = std::pow( 2, i );
         size_type const depth_block_count = block_count * depth_pow;
         size_type const depth_block_size = block_size / depth_pow;

         true_alloc_size += depth_block_count * ( sizeof( block_header ) + depth_block_size );
      }

      p_memory = std::make_unique<std::byte[]>( true_alloc_size + ( ( pool_depth + 1 ) * sizeof( access_header ) ) );

      p_access_headers = TO_ACCESS_HEADER_PTR( p_memory.get( ) );
      for ( int i = 0; i < pool_depth; ++i )
      {
         size_type const depth_pow = std::pow( 2, i );
         size_type const depth_offset = ( pool_depth + 1 ) * sizeof( block_header );
         size_type const pool_offset = block_count * block_size * i;

         p_access_headers[i].p_first_free = TO_BLOCK_HEADER_PTR( p_memory.get( ) + depth_offset + pool_offset );

         block_header* p_first_free = p_access_headers[i].p_first_free;
         p_first_free->depth_index = i;

         for ( int j = 1; j < block_count * depth_pow; ++j )
         {
            size_type const offset = j * ( block_size / depth_pow + sizeof( block_header ) );

            auto* p_new = TO_BLOCK_HEADER_PTR( p_memory.get( ) + offset + depth_offset + pool_offset );
            p_first_free->p_next = p_new;
            p_first_free = p_new;
            p_first_free->p_next = nullptr;
         }
      }
   }

   auto multipool_allocator::allocate( size_type size, size_type alignment ) noexcept -> pointer
   {
      assert( size != 0 && "Allocation size cannot be zero" );
      assert( size <= block_size && "Allocation size cannot be greater than max pool size" );
      assert( alignment != 0 && "Allocation alignment cannot be zero" );

      size_type depth_index = 0;
      for ( int i = 0; i < pool_depth; ++i )
      {
         if ( block_size / std::pow( 2, i ) == size )
         {
            depth_index = i;
            break;
         }
      }

      if ( p_access_headers[depth_index].p_first_free )
      {
         block_header* p_block_header = p_access_headers[depth_index].p_first_free;
         p_access_headers[depth_index].p_first_free = p_block_header->p_next;

         used_memory += block_size;
         ++num_allocations;

         return TO_BYTE_PTR( ++p_block_header );
      }
      else
      {
         return nullptr;
      }
   }

   void multipool_allocator::free( pointer p_alloc ) noexcept
   {
      assert( p_alloc != nullptr && "cannot free a nullptr" );

      auto* p_header = reinterpret_cast<block_header*>( p_alloc - sizeof( block_header ) );
      p_header->p_next = p_access_headers[p_header->depth_index].p_first_free;
      p_access_headers[p_header->depth_index].p_first_free = p_header;

      used_memory -= block_size;
      --num_allocations;
   }

   bool multipool_allocator::can_allocate( size_type size, size_type alignment ) const noexcept
   {
      assert( size != 0 && "Size cannot be zero." );
      assert( size <= block_size && "Sblock_size / depth_powize cannot be greater than max pool size" );
      assert( alignment != 0 && "Alignment cannot be zero" );

      auto const depth_index = std::clamp( block_size / size, size_type{1}, pool_depth ) - 1;

      return p_access_headers[depth_index].p_first_free != nullptr;
   }

   auto multipool_allocator::allocation_capacity( pointer p_alloc ) const noexcept -> size_type
   {
      assert( p_alloc != nullptr && "Cannot access nullptr" );

      auto* p_header = TO_BLOCK_HEADER_PTR( p_alloc );
      --p_header;

      return block_size / std::pow( 2, p_header->depth_index );
   }

   void multipool_allocator::clear( ) noexcept
   {
      p_access_headers = reinterpret_cast<access_header*>( p_memory.get( ) );
      for ( int i = 0; i < pool_depth; ++i )
      {
         size_type const depth_pow = std::pow( 2, i );
         size_type const depth_offset = ( pool_depth + 1 ) * sizeof( block_header );
         size_type const pool_offset = block_count * block_size * i;

         p_access_headers[i].p_first_free = TO_BLOCK_HEADER_PTR( p_memory.get( ) + depth_offset + pool_offset );

         auto* p_base_cpy = p_access_headers[i].p_first_free;
         for ( int j = 1; j < block_count * depth_pow; ++j )
         {
            size_type const offset = j * ( block_size / depth_pow + sizeof( block_header ) );

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
