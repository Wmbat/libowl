#include <ESL/allocators/multipool_allocator.hpp>
#include <ESL/containers/vector.hpp>

#include <vector>

int main( )
{
   auto multipool = ESL::multipool_allocator{1, 2048, 2};
   ESL::vector<int, ESL::multipool_allocator> my_vec{2048 / sizeof( int ), 10, &multipool};

   my_vec.assign( 20, 20 );

   ESL::vector<int, ESL::multipool_allocator> my_vec2{1024 / sizeof( int ), 10, &multipool};
   my_vec2.assign( 2048 / sizeof( int ), 40 );

   return 0;
}
