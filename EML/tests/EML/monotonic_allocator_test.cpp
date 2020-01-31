#include <EML/monotonic_allocator.hpp>

#include <gtest/gtest.h>

struct monotonic_allocator_test : public testing::Test
{
   monotonic_allocator_test( ) : my_allocator( 1024 ) {}

   EML::monotonic_allocator my_allocator;
};

TEST_F( monotonic_allocator_test, make_new_inplace_args_test )
{
   auto* test_alloc = my_allocator.make_new<int>( 10 );

   EXPECT_NE( nullptr, test_alloc );
   EXPECT_EQ( 10, *test_alloc );
}

TEST_F( monotonic_allocator_test, make_new_assigment_test )
{
   auto* test_alloc = my_allocator.make_new<int>( );

   EXPECT_NE( nullptr, test_alloc );

   *test_alloc = 10;
   EXPECT_EQ( 10, *test_alloc );
}

TEST_F( monotonic_allocator_test, over_capacity_test )
{
   EXPECT_EQ( nullptr, my_allocator.allocate( 1050, sizeof( std::size_t ) ) );
}

TEST_F( monotonic_allocator_test, clearing_test )
{
   my_allocator.clear( );
   
   auto* test_alloc_1 = my_allocator.allocate( 1000, alignof( std::size_t ) );

   EXPECT_NE( nullptr, test_alloc_1 );

   my_allocator.clear( );

   auto* test_alloc_2 = my_allocator.allocate( 1000, alignof( std::size_t ) );

   EXPECT_NE( nullptr, test_alloc_2 );
}
