#include <ELL/logger.hpp>

#include <EML/multipool_allocator.hpp>

int main( )
{
   EML::multipool_allocator my_allocator{1, 1024, 2};
   auto test = my_allocator.max_size( );

   auto one = my_allocator.allocate( 1024, alignof( int ) );
   auto two = my_allocator.allocate( 1024, alignof( int ) );
   auto three = my_allocator.allocate( 1024 / 2, alignof( int ) );

   return 0;
}
