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

#include <ESL/allocators/pool_allocator.hpp>

#include <utility>

namespace ESL
{
   pool_allocator::pool_allocator( std::size_t const block_count, std::size_t const block_size ) noexcept :
      total_size( block_count * block_size ), used_memory( 0 ), num_allocations( 0 ), block_count( block_count ),
      block_size( block_size ),
      p_memory( std::make_unique<std::byte[]>( block_count * ( block_size + sizeof( block_header ) ) ) ),
      p_first_free( nullptr )
   {
      assert( block_count != 0 && "Cannot have no blocks in memory pool" );
      assert( block_size != 0 && "Cannot have a block size of zero" );

      p_first_free = reinterpret_cast<block_header*>( p_memory.get( ) );
      auto* p_base_cpy = p_first_free;

      for ( int i = 1; i < block_count; ++i )
      {
         std::size_t offset = i * ( block_size + sizeof( block_header ) );

         auto* p_new = reinterpret_cast<block_header*>( p_memory.get( ) + offset );
         p_base_cpy->p_next = p_new;
         p_base_cpy = p_new;
         p_base_cpy->p_next = nullptr;
      }
   }

   std::byte* pool_allocator::allocate( std::size_t size, std::size_t alignment ) noexcept
   {
      assert( size != 0 && "Allocation size cannot be zero" );
      assert( alignment != 0 && "Allocation alignment cannot be zero" );

      if ( p_first_free )
      {
         std::byte* p_chunk_header = reinterpret_cast<std::byte*>( p_first_free );

         p_first_free = p_first_free->p_next;

         used_memory += block_size;
         ++num_allocations;

         return reinterpret_cast<std::byte*>( p_chunk_header + sizeof( block_header ) );
      }
      else
      {
         return nullptr;
      }
   }

   void pool_allocator::free( std::byte* p_location ) noexcept
   {
      assert( p_location != nullptr && "cannot free a nullptr" );

      auto* p_header = reinterpret_cast<block_header*>( p_location - sizeof( block_header ) );
      p_header->p_next = p_first_free;
      p_first_free = p_header;

      used_memory -= block_size;
      --num_allocations;
   }

   void pool_allocator::clear( ) noexcept {}

   std::size_t pool_allocator::max_size( ) const noexcept { return total_size; }
   std::size_t pool_allocator::memory_usage( ) const noexcept { return used_memory; }
   std::size_t pool_allocator::allocation_count( ) const noexcept { return num_allocations; }
} // namespace EML
