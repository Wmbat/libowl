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

#include <ESL/allocators/multipool_allocator.hpp>

#include <gtest/gtest.h>

struct multipool_allocator_test : public testing::Test
{
   static constexpr std::size_t max_size = sizeof( std::size_t[16] );

   multipool_allocator_test( ) : my_allocator( 1, max_size, 2 ) {}

   ESL::multipool_allocator my_allocator;
};

TEST_F( multipool_allocator_test, size_test )
{
   EXPECT_EQ( max_size * 2, my_allocator.max_size( ) );
}

TEST_F( multipool_allocator_test, basic_alloc_n_assign_test )
{
   auto* p_alloc = my_allocator.allocate( max_size, alignof( int ) );

   EXPECT_NE( nullptr, p_alloc );

   auto* p_data = new ( p_alloc ) int( 10 );

   EXPECT_NE( nullptr, p_data );
   EXPECT_EQ( 10, *p_data );
}

TEST_F( multipool_allocator_test, max_alloc_test )
{
   auto* p_first_alloc = my_allocator.allocate( max_size, alignof( int ) );

   EXPECT_EQ( 1, my_allocator.allocation_count( ) );

   auto* p_second_alloc = my_allocator.allocate( max_size, alignof( int ) );

   EXPECT_EQ( 1, my_allocator.allocation_count( ) );

   auto* p_third_alloc = my_allocator.allocate( max_size / 2, alignof( int ) );

   EXPECT_EQ( 2, my_allocator.allocation_count( ) );

   auto* p_fourth_alloc = my_allocator.allocate( max_size / 2, alignof( int ) );

   EXPECT_EQ( 3, my_allocator.allocation_count( ) );

   auto* p_fifth_alloc = my_allocator.allocate( max_size / 2, alignof( int ) );

   EXPECT_EQ( 3, my_allocator.allocation_count( ) );

   EXPECT_NE( nullptr, p_first_alloc );
   EXPECT_EQ( nullptr, p_second_alloc );
   EXPECT_NE( nullptr, p_third_alloc );
   EXPECT_NE( nullptr, p_fourth_alloc );
   EXPECT_EQ( nullptr, p_fifth_alloc );
}

TEST_F( multipool_allocator_test, make_new_test )
{
   struct my_data
   {
      std::size_t data[16];
   };

   auto* p_data = my_allocator.make_new<my_data>( );

   EXPECT_NE( nullptr, p_data );
}

TEST_F( multipool_allocator_test, free_test )
{
   auto* p_first_alloc = my_allocator.allocate( max_size, alignof( int ) );
   auto* p_null_alloc = my_allocator.allocate( max_size, alignof( int ) );

   EXPECT_NE( nullptr, p_first_alloc );
   EXPECT_EQ( nullptr, p_null_alloc );

   my_allocator.free( p_first_alloc );

   auto* p_second_alloc = my_allocator.allocate( max_size, alignof( int ) );

   EXPECT_NE( nullptr, p_second_alloc );
}

TEST_F( multipool_allocator_test, clear_test )
{
   struct my_data
   {
      std::size_t data[16];
   };

   auto data_one = my_allocator.make_new<my_data>( );

   EXPECT_EQ( my_allocator.allocation_count( ), 1 );
   EXPECT_EQ( my_allocator.memory_usage( ), sizeof( my_data ) );

   my_allocator.clear( );

   EXPECT_EQ( my_allocator.allocation_count( ), 0 );
   EXPECT_EQ( my_allocator.memory_usage( ), 0 );

   auto* p_data_two = my_allocator.make_new<my_data>( );

   EXPECT_NE( p_data_two, nullptr );
   EXPECT_EQ( my_allocator.allocation_count( ), 1 );
   EXPECT_EQ( my_allocator.memory_usage( ), sizeof( my_data ) );
}

TEST_F( multipool_allocator_test, array_alloc_test )
{
   auto p_arr = my_allocator.make_array<std::size_t>( 16 );

   EXPECT_NE( p_arr, nullptr );
   EXPECT_EQ( my_allocator.allocation_count( ), 1 );

   for ( std::size_t i = 0; i < 16; ++i )
   {
      p_arr[i] = 10;
      EXPECT_EQ( p_arr[i], 10 );
   }

   my_allocator.make_delete( p_arr, 16 );

   EXPECT_EQ( my_allocator.allocation_count( ), 0 );
}
