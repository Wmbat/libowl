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

#include <epona_library/allocators/allocator_utils.hpp>

namespace ESL
{
   /**
    * @class monotonic_allocator monotonic_allocator.hpp <ESL/allocators/monotonic_allocator.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Tuesday April 7th, 2020
    * @brief A simple allocator where memory is only released upon destruction of the allocator.
    * @copyright MIT License
    */
   class monotonic_allocator final
   {
   public:
      using pointer = void*;
      using size_type = std::size_t;

   public:
      /**
       * @brief Constructs a #monotonic_allocator where the buffer is set no null.
       */
      monotonic_allocator( ) = default;
      /**
       * @brief Constructs a monotonic_allocator with size initial_size.
       *
       * @param[in]  initial_size   The initial size of the memory allocation.
       */
      explicit monotonic_allocator( size_type initial_size ) noexcept;

      /**
       * @brief Give out a block of memory from the allocator
       *
       * @param[in]  size        The size in bytes of the memory block needed.
       * @param[in]  alignment   The alignment of the memory block needed.
       *
       * @return A pointer to the memory block.
       */
      [[nodiscard( "Memory will go to waste" )]] pointer allocate(
         size_type size, size_type alignment = alignof( std::max_align_t ) ) noexcept;
      /**
       * @brief Does nothing.
       *
       * @param[in]  p_alloc     Pointer to the memory block previously acquired.
       */
      void deallocate( pointer* p_alloc ) noexcept;

      /**
       * @brief Reset the allocator's memory. All pointers received from the allocator become invalid.
       */
      void release( ) noexcept;

      /**
       * @brief Return the size of the allocator's memory.
       *
       * @return The size of the allocator's memory.
       */
      size_type max_size( ) const noexcept;
      /**
       * @brief Return the amount of memory that has been given out.
       */
      size_type memory_usage( ) const noexcept;
      /**
       * @brief Return the amount of times memory has been given out.
       */
      size_type allocation_count( ) const noexcept;

   private:
      size_type total_size{ 0 };
      size_type used_memory{ 0 };
      size_type num_allocations{ 0 };

      std::unique_ptr<std::byte[]> p_memory{ nullptr };
      std::byte* p_current_pos{ nullptr };
   };
} // namespace ESL
