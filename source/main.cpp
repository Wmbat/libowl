#include <ESL/allocators/pool_allocator.hpp>
#include <ESL/containers/vector.hpp>

struct copyable
{
   copyable( ) = default;
   copyable( int i ) : i( i ) {}

   int i = 0;
};

int main( )
{
   using vector = ESL::vector<copyable, ESL::pool_allocator>;

   ESL::pool_allocator main_pool( 2, 2048 );

   copyable test{20};

   auto [vec_opt, err_code] = vector::make( 10, test, &main_pool );

   return 0;
}
