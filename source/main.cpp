#include <ELL/logger.hpp>

#include <EML/multipool_allocator.hpp>

int main( )
{
   EML::multipool_allocator my_allocator{1, 1024, 2};
   auto test = my_allocator.max_size( );

   auto* p_one = my_allocator.allocate( 1024, alignof( int ) );
   auto* p_two = my_allocator.allocate( 1024, alignof( int ) );
   auto* p_three = my_allocator.allocate( 1024 / 2, alignof( int ) );

   return 0;
}
