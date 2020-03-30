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
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <ESL/allocators/allocator_utils.hpp>
#include <ESL/allocators/multipool_allocator.hpp>

#include <concepts>

namespace ESL
{
   template <class key_, class value_, class compare_ = std::less<key_>, allocator allocator_ = multipool_allocator>
   class avl_map
   {
   public:
      using key_type = key_;
      using mapped_type = value_;
      using value_type = std::pair<key_type const, mapped_type>;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using key_compare = compare_;
      using reference = value_type&;
      using const_reference = value_type const&;

   public:
      explicit avl_map( allocator_* p_allocator ) {}
      avl_map( std::input_iterator auto first, std::input_iterator auto last, allocator_* p_allocator ) {}

   private:
      key_compare comp{ };
      allocator_* p_allocator{ nullptr };
   };
} // namespace ESL
