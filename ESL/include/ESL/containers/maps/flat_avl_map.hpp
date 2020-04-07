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

#include <ESL/allocators/multipool_allocator.hpp>
#include <ESL/utils/concepts.hpp>
#include <ESL/utils/iterators/bidirectional_iterator.hpp>

#include <concepts>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>

namespace ESL
{
   /**
    * \class flat_avl_map flat_avl_map.hpp <ESL/containers/maps/flat_avl_map.hpp>
    * \brief
    */
   template <std::equality_comparable key_, class any_, allocator allocator_ = multipool_allocator,
      class compare_ = std::less<key_>>
   class flat_avl_map
   {
   public:
      using key_type = key_;
      using mapped_type = any_;
      using value_type = std::pair<key_ const, any_>;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using key_compare = compare_;
      using reference = value_type&;
      using const_reference = value_type const&;
      using pointer = value_type*;
      using const_pointer = value_type const*;
      using iterator = bidirectional_iterator<value_type>;
      using const_iterator = bidirectional_iterator<value_type const>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      explicit flat_avl_map( allocator_ const* const p_allocator ) : p_allocator( p_allocator ) {}
      flat_avl_map(
         std::input_iterator auto first, std::input_iterator auto last, allocator_ const* const p_allocator ) :
         p_allocator( p_allocator )
      {}
      flat_avl_map( std::initializer_list<value_type> init, allocator_ const* const p_allocator ) :
         p_allocator( p_allocator )
      {}
      flat_avl_map( flat_avl_map const& other ) {}
      flat_avl_map( flat_avl_map const& other, allocator_ const* const p_allocator ) {}
      flat_avl_map( flat_avl_map&& other ) {}
      flat_avl_map( flat_avl_map&& other, allocator_ const* const p_allocator ) {}
      ~flat_avl_map( ) {}

      // Modifies
      /**
       *
       */
      std::pair<iterator, bool> insert( const value_type& value ) {}
      std::pair<iterator, bool> insert( value_type&& value ) {}
      template <class value_>
      std::pair<iterator, bool> insert( std::constructible_from<value_type, value_&&> auto&& value )
      {}

   private:
   private:
      key_compare compare{ };

      allocator_* p_allocator{ nullptr };
      value_type* p_data{ nullptr };

      size_type count{ 0 };
      size_type cap{ 0 };
   };
} // namespace ESL
