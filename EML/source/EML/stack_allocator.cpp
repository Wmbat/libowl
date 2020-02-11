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

#include <EML/stack_allocator.hpp>

#include <cassert>

namespace EML
{
   stack_allocator::stack_allocator( std::size_t const size ) noexcept :
      allocator_interface( size ), p_memory( std::make_unique<std::byte[]>( size ) ), p_top( p_memory.get( ) )
   {}

   std::byte* stack_allocator::allocate( std::size_t size, std::size_t alignment ) noexcept
   {
      assert( size != 0 );

      auto const padding = get_forward_padding( reinterpret_cast<std::uintptr_t>( p_top ), alignment, sizeof( header ) );

      if ( padding + size + used_memory > total_size )
      {
         return nullptr;
      }

      std::byte* aligned_address = p_top + padding;

      auto* header_address = reinterpret_cast<header*>( aligned_address - sizeof( header ) );
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

   void stack_allocator::clear( ) noexcept
   {
      p_top = p_memory.get( );
      used_memory = 0;
      num_allocations = 0;
   }
} // namespace EML
