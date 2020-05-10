#include <epona_library/containers/flat_map.hpp>
#include <epona_library/allocators/pool_allocator.hpp>

struct tiny_flat_avl_map_test : public testing::Test
{
   tiny_flat_avl_map_test( ) :
      pool_allocator( { .pool_count = 2, .pool_size = 2048 } ),
      secondary_allocator( { .pool_count = 2, .pool_size = 2048 } )
   {}

   ESL::pool_allocator pool_allocator;
   ESL::pool_allocator secondary_allocator;
};

TEST_F( tiny_flat_avl_map_test, default_ctor )
{
}
