#include <ESL/allocators/multipool_allocator.hpp>
#include <ESL/containers/vector.hpp>

int main( )
{
   auto pool = ESL::multipool_allocator{1, 1024, 1};
   auto vec = ESL::vector<int, decltype( pool )>{&pool};
   vec.emplace_back( 1 );
   vec.emplace_back( 2 );
   vec.emplace_back( 3 );
   vec.emplace_back( 4 );
   vec.emplace_back( 5 );

   vec.erase( vec.cbegin( ) + 2 );

   auto i = *( vec.begin( ) );

   return 0;
}
