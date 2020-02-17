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

#include <ESL/allocators/monotonic_allocator.hpp>

#include <gtest/gtest.h>

struct monotonic_allocator_test : public testing::Test
{
   monotonic_allocator_test( ) : my_allocator( 1024 ) {}

   ESL::monotonic_allocator my_allocator;
};

TEST_F( monotonic_allocator_test, size_test )
{
   EXPECT_EQ( my_allocator.max_size( ), 1024 );
}

TEST_F( monotonic_allocator_test, basic_alloc_n_assign_test )
{
   auto* p_alloc = my_allocator.allocate( 1024, alignof( int ) );

   EXPECT_NE( p_alloc, nullptr );

   auto* p_data = new ( p_alloc ) int( 10 );

   EXPECT_NE( p_data, nullptr );
   EXPECT_EQ( *p_data, 10 );
}

TEST_F( monotonic_allocator_test, over_allocation_test )
{
   auto* p_alloc = my_allocator.allocate( 2000, alignof( int ) );

   EXPECT_EQ( p_alloc, nullptr );
}

TEST_F( monotonic_allocator_test, make_new_test )
{
   auto* p_data = my_allocator.make_new<int>( 10 );

   EXPECT_NE( p_data, nullptr );
   EXPECT_EQ( *p_data, 10 );
}

TEST_F( monotonic_allocator_test, allocation_count_test )
{
   EXPECT_EQ( my_allocator.allocation_count( ), 0 );

   auto* p_data = my_allocator.make_new<int>( 10 );

   EXPECT_NE( p_data, nullptr );
   EXPECT_EQ( *p_data, 10 );
   EXPECT_EQ( my_allocator.allocation_count( ), 1 );

   auto* p_data_two = my_allocator.make_new<int>( 10 );

   EXPECT_NE( p_data_two, nullptr );
   EXPECT_EQ( *p_data_two, 10 );
   EXPECT_EQ( my_allocator.allocation_count( ), 2 );

   my_allocator.clear( );

   EXPECT_EQ( my_allocator.allocation_count( ), 0 );
}

TEST_F( monotonic_allocator_test, array_alloc_test )
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
   EXPECT_EQ( my_allocator.memory_usage( ), sizeof( int ) * elem_count );

   my_allocator.clear( );

   EXPECT_EQ( my_allocator.allocation_count( ), 0 );
   EXPECT_EQ( my_allocator.memory_usage( ), 0 );
}
