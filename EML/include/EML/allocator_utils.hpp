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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <functional>

namespace EML
{
   template <class type_>
   using auto_ptr = std::unique_ptr<type_, std::function<void( type_* )>>;

   constexpr std::size_t get_backward_padding( std::uintptr_t address, std::size_t aligment ) noexcept
   {
      auto const padding = address & ( aligment - 1 );

      return padding == aligment ? 0 : padding;
   }

   constexpr std::size_t get_forward_padding( std::uintptr_t address, std::size_t alignment ) noexcept
   {
      auto const padding = alignment - ( address & ( alignment - 1 ) );

      return padding == alignment ? 0 : padding;
   }

   constexpr std::size_t get_forward_padding(
      std::uintptr_t address, std::size_t alignment, std::size_t header_size ) noexcept
   {
      auto padding = get_forward_padding( address, alignment );

      if ( padding < header_size )
      {
         auto const needed_space = header_size - padding;

         padding += alignment * ( needed_space / alignment );
         if ( needed_space % alignment > 0 )
         {
            padding += alignment;
         }
      }

      return padding;
   }
} // namespace EML
