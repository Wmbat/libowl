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
