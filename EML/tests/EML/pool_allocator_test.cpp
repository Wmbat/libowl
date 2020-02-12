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

#include <EML/pool_allocator.hpp>

#include <gtest/gtest.h>
#include <vector>

struct pool_allocator_test : public testing::Test
{
   pool_allocator_test( ) : my_allocator( 2, alloc_size ) {}

   constexpr static std::size_t alloc_size = 1024;
   EML::pool_allocator my_allocator;
};

TEST_F( pool_allocator_test, simple_allocation_test )
{
   auto* test_alloc = my_allocator.allocate( alloc_size, alignof( bool ) );

   EXPECT_NE( nullptr, test_alloc );
}

TEST_F( pool_allocator_test, over_allocation_test )
{
   auto* p_first_alloc = my_allocator.allocate( alloc_size, alignof( EML::pool_allocator ) );
   auto* p_second_alloc = my_allocator.allocate( alloc_size, alignof( std::vector<float> ) );
   auto* p_third_alloc = my_allocator.allocate( alloc_size, alignof( bool[2] ) );

   EXPECT_NE( nullptr, p_first_alloc );
   EXPECT_NE( nullptr, p_second_alloc );
   EXPECT_EQ( nullptr, p_third_alloc );
}

TEST_F( pool_allocator_test, simple_free_test )
{
   auto* p_first_alloc = my_allocator.allocate( alloc_size, alignof( EML::pool_allocator ) );
   auto* p_second_alloc = my_allocator.allocate( alloc_size, alignof( std::vector<float> ) );

   EXPECT_NE( nullptr, p_first_alloc );
   EXPECT_NE( nullptr, p_second_alloc );

   my_allocator.free( p_second_alloc );
   p_second_alloc = nullptr;

   auto* p_third_alloc = my_allocator.allocate( alloc_size, alignof( bool[2] ) );

   EXPECT_NE( nullptr, p_third_alloc );
}

TEST_F( pool_allocator_test, unique_ptr_alloc_test )
{
   auto p_first_alloc = my_allocator.make_unique<int>( 3 );

   EXPECT_NE( nullptr, p_first_alloc.get( ) );

   {
      auto p_second_alloc = my_allocator.make_unique<int>( 3 );

      EXPECT_NE( nullptr, p_second_alloc.get( ) );

      auto p_third_alloc = my_allocator.make_unique<int>( 3 );

      EXPECT_EQ( nullptr, p_third_alloc.get( ) );
   }

   auto p_fourth_alloc = my_allocator.make_unique<int>( 3 );

   EXPECT_NE( nullptr, p_fourth_alloc.get( ) );
}

TEST_F( pool_allocator_test, array_alloc_test )
{
   std::size_t const elem_count = 10;
   auto* p_array = my_allocator.make_array<int>( elem_count );

   EXPECT_NE( p_array, nullptr );

   for ( std::size_t i = 0; i < elem_count; ++i )
   {
      p_array[i] = 10;
      EXPECT_EQ( p_array[i], 10 );
   }

   EXPECT_EQ( my_allocator.allocation_count( ), 1 );

   my_allocator.make_delete( p_array, elem_count );

   EXPECT_EQ( my_allocator.allocation_count( ), 0 );
}
