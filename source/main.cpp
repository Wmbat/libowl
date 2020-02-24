#include <ESL/allocators/multipool_allocator.hpp>
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
   ESL::pool_allocator main_pool_alloc{4, 2048};

   copyable test{20};
   ESL::vector<copyable, ESL::pool_allocator> my_vec{10, test, &main_pool_alloc};

   for ( auto& it : my_vec )
   {
      it.i = 10;
   }

   decltype( my_vec ) my_vec2{my_vec.begin( ), my_vec.end( ), &main_pool_alloc};

   for ( auto const& it : my_vec2 )
   {
      int test = it.i;
   }

   return 0;
}
