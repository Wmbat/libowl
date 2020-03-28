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

#include <ESL/allocators/stack_allocator.hpp>

#include <cassert>

#define TO_UINT_PTR( ptr ) reinterpret_cast<std::uintptr_t>( ptr )
#define TO_HEADER_PTR( ptr ) reinterpret_cast<header*>( ptr )

namespace ESL
{
   stack_allocator::stack_allocator( size_type const size ) noexcept :
      total_size( size ), used_memory( 0 ), num_allocations( 0 ), p_memory( std::make_unique<std::byte[]>( size ) ),
      p_top( p_memory.get( ) )
   {}

   std::byte* stack_allocator::allocate( size_type size, size_type alignment ) noexcept
   {
      assert( size != 0 );

      auto const padding = get_forward_padding( TO_UINT_PTR( p_top ), alignment, sizeof( header ) );

      if ( padding + size + used_memory > total_size )
      {
         return nullptr;
      }

      std::byte* aligned_address = p_top + padding;

      auto* header_address = TO_HEADER_PTR( aligned_address - sizeof( header ) );
      header_address->adjustment = padding;

      p_top = aligned_address + size;

      used_memory += size + padding;
      ++num_allocations;

      return aligned_address;
   }

   void stack_allocator::free( std::byte* p_address ) noexcept
   {
      assert( p_address != nullptr );

      auto* header_address = reinterpret_cast<header*>( p_address - sizeof( header ) );
      used_memory -= p_top - p_address + header_address->adjustment;

      p_top = p_address - header_address->adjustment;

      --num_allocations;
   }

   bool stack_allocator::can_allocate( size_type size, size_type alignment ) const noexcept
   {
      assert( size != 0 && "Size cannot be zero" );
      assert( alignment != 0 && "Alignment cannot be zero" );

      auto const padding = get_forward_padding( TO_UINT_PTR( p_top ), alignment, sizeof( header ) );
      return ( padding + size + used_memory ) > total_size;
   }

   void stack_allocator::clear( ) noexcept
   {
      p_top = p_memory.get( );
      used_memory = 0;
      num_allocations = 0;
   }

   stack_allocator::size_type stack_allocator::max_size( ) const noexcept { return total_size; }
   stack_allocator::size_type stack_allocator::memory_usage( ) const noexcept { return used_memory; }
   stack_allocator::size_type stack_allocator::allocation_count( ) const noexcept { return num_allocations; }
} // namespace ESL

#undef TO_UINT_PTR
#undef TO_HEADER_PTR
