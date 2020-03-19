
#include <ESL/allocators/pool_allocator.hpp>
#include <ESL/containers/vector.hpp>

struct copyable
{
   int i;
};

int main( )
{
   auto main_pool_alloc = ESL::pool_allocator{ 2, 2048 };
   ESL::vector<copyable, ESL::pool_allocator> my_cpy_vec{ &main_pool_alloc };

   copyable test{ 10 };
   my_cpy_vec.assign( 10, test );

   ESL::vector<copyable, ESL::pool_allocator> my_vec{ &main_pool_alloc };
   my_vec.assign( my_cpy_vec.cbegin( ), my_cpy_vec.cend( ) );

   ESL::vector<copyable, ESL::pool_allocator> my_vec2{ &main_pool_alloc };
   my_vec2.assign( my_vec.cbegin( ), my_vec.cend( ) );

   return 0;
}
