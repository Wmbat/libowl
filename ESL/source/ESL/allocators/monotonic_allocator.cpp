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

#include <ESL/allocators/monotonic_allocator.hpp>

#include <cassert>

#define TO_UINT_PTR( ptr ) reinterpret_cast<std::uintptr_t>( ptr )

namespace ESL
{
   monotonic_allocator::monotonic_allocator( std::size_t size ) noexcept :
      total_size( size ), num_allocations( 0 ), used_memory( 0 ), p_memory( std::make_unique<std::byte[]>( size ) ),
      p_current_pos( p_memory.get( ) )
   {}

   std::byte* monotonic_allocator::allocate( std::size_t size, std::size_t alignment ) noexcept
   {
      assert( size != 0 );

      auto const padding = get_forward_padding( TO_UINT_PTR( p_current_pos ), alignment );

      if ( padding + size + used_memory > total_size )
      {
         return nullptr;
      }

      std::byte* aligned_address = p_current_pos + padding;
      p_current_pos = aligned_address + size;

      used_memory += size + padding;
      ++num_allocations;

      return aligned_address;
   }

   bool monotonic_allocator::can_allocate( size_type size, size_type alignment ) const noexcept
   {
      assert( size != 0 );

      auto const padding = get_forward_padding( TO_UINT_PTR( p_current_pos ), alignment );

      return ( padding + size + used_memory ) > total_size;
   }

   void monotonic_allocator::clear( ) noexcept
   {
      p_current_pos = p_memory.get( );
      used_memory = 0;
      num_allocations = 0;
   }

   std::size_t monotonic_allocator::max_size( ) const noexcept { return total_size; }
   std::size_t monotonic_allocator::memory_usage( ) const noexcept { return used_memory; }
   std::size_t monotonic_allocator::allocation_count( ) const noexcept { return num_allocations; }
} // namespace ESL
