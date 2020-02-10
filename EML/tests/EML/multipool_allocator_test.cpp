#include <EML/multipool_allocator.hpp>

#include <gtest/gtest.h>

struct multipool_allocator_test : public testing::Test
{
   multipool_allocator_test( ) : my_allocator( 1, 128, 2 ) {}

   EML::multipool_allocator my_allocator;
};

TEST_F( multipool_allocator_test, size_test )
{
   EXPECT_EQ( 128 * 2, my_allocator.max_size() );
}

TEST_F( multipool_allocator_test, basic_alloc_n_assign_test )
{
   auto* p_alloc = my_allocator.allocate( 128, alignof( int ) );

   EXPECT_NE( nullptr, p_alloc );

   auto* p_data = new ( p_alloc ) int( 10 );
   
   EXPECT_NE( nullptr, p_data );
   EXPECT_EQ( 10, *p_data );
}

TEST_F( multipool_allocator_test, max_alloc_test )
{
   auto* p_first_alloc = my_allocator.allocate( 128, alignof( int ) );

   EXPECT_EQ( 1, my_allocator.allocation_count( ) );

   auto* p_second_alloc = my_allocator.allocate( 128, alignof( int ) );

   EXPECT_EQ( 1, my_allocator.allocation_count( ) );

   auto* p_third_alloc = my_allocator.allocate( 128 / 2, alignof( int ) );
   
   EXPECT_EQ( 2, my_allocator.allocation_count( ) );
   
   auto* p_fourth_alloc = my_allocator.allocate( 128 / 2, alignof( int ) );

   EXPECT_EQ( 3, my_allocator.allocation_count( ) );

   auto* p_fifth_alloc = my_allocator.allocate( 128 / 2, alignof( int ) );

   EXPECT_EQ( 3, my_allocator.allocation_count( ) );

   EXPECT_NE( nullptr, p_first_alloc );
   EXPECT_EQ( nullptr, p_second_alloc );
   EXPECT_NE( nullptr, p_third_alloc );
   EXPECT_NE( nullptr, p_fourth_alloc );
   EXPECT_EQ( nullptr, p_fifth_alloc );
}
