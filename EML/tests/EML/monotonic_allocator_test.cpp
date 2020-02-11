#include <EML/monotonic_allocator.hpp>

#include <gtest/gtest.h>

struct monotonic_allocator_test : public testing::Test
{
   monotonic_allocator_test( ) : my_allocator( 1024 ) {}

   EML::monotonic_allocator my_allocator;
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
