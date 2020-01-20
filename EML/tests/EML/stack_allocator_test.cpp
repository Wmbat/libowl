#include <EML/allocators/stack_allocator.hpp>

#include <gtest/gtest.h>

struct stack_allocator_test : public testing::Test
{
   stack_allocator_test( ) {}

   EML::stack_allocator<1024> allocator;
};

TEST_F( stack_allocator_test, make_new_inplace_args_test )
{
   auto* test_alloc = allocator.make_new<int>( 10 );

   EXPECT_NE( nullptr, test_alloc );
   EXPECT_EQ( 10, *test_alloc );
}
