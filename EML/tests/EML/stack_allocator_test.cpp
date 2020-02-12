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

#include <gtest/gtest.h>

struct stack_allocator_test : public testing::Test
{
   stack_allocator_test( ) : my_allocator( 1024 ) {}

   EML::stack_allocator my_allocator;
};

TEST_F( stack_allocator_test, make_new_inplace_args_test )
{
   auto* test_alloc = my_allocator.make_new<int>( 10 );

   EXPECT_NE( nullptr, test_alloc );
   EXPECT_EQ( 10, *test_alloc );
}

TEST_F( stack_allocator_test, over_capacity_test )
{
   EXPECT_EQ( nullptr, my_allocator.allocate( 1050, alignof( std::size_t ) ) );
}

TEST_F( stack_allocator_test, deletion_test )
{
   auto* p_alloc_1 = my_allocator.make_new<int>( 10 );

   EXPECT_NE( nullptr, p_alloc_1 );
   EXPECT_EQ( 10, *p_alloc_1 );

   auto* p_alloc_2 = my_allocator.make_new<int>( 20 );

   EXPECT_NE( nullptr, p_alloc_2 );
   EXPECT_EQ( 20, *p_alloc_2 );

   my_allocator.make_delete( p_alloc_2 );
}

TEST_F( stack_allocator_test, array_creation_test )
{
   std::size_t elem_count = 20;
   auto* p_array = my_allocator.make_array<int>( elem_count );

   EXPECT_NE( p_array, nullptr );

   for ( int i = 0; i < elem_count; ++i )
   {
      p_array[i] = 10;
      EXPECT_EQ( p_array[i], 10 );
   }

   my_allocator.make_delete( p_array, elem_count );
}
