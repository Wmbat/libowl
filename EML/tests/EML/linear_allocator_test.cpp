#include <EML/allocators/linear_allocator.hpp>

#include <gtest/gtest.h>

struct linear_allocator_test : public testing::Test
{
   linear_allocator_test( ) {}

   EML::linear_allocator<1024> allocator;
};

TEST_F( linear_allocator_test, make_new_inplace_args_test )
{
   auto* test_alloc = allocator.make_new<int>( 10 );

   EXPECT_NE( nullptr, test_alloc );
   EXPECT_EQ( 10, *test_alloc );
}

TEST_F( linear_allocator_test, make_new_assigment_test )
{
   auto* test_alloc = allocator.make_new<int>( );

   EXPECT_NE( nullptr, test_alloc );

   *test_alloc = 10;
   EXPECT_EQ( 10, *test_alloc );
}

TEST_F( linear_allocator_test, over_capacity_test )
{
   EXPECT_EQ( nullptr, allocator.allocate( 1050, sizeof( std::size_t ) ) );
}

TEST_F( linear_allocator_test, clearing_test )
{
   allocator.clear( );
   
   auto* test_alloc_1 = allocator.allocate( 1000, alignof( std::size_t ) );

   EXPECT_NE( nullptr, test_alloc_1 );

   allocator.clear( );

   auto* test_alloc_2 = allocator.allocate( 1000, alignof( std::size_t ) );

   EXPECT_NE( nullptr, test_alloc_2 );
}
