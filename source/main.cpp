#include <ELL/logger.hpp>

#include <EML/pool_allocator.hpp>

int main( )
{
   auto alloc_size = 1024;
   EML::pool_allocator my_allocator{2, 1024};

   auto* p_first_alloc = my_allocator.allocate( alloc_size, alignof( EML::pool_allocator ) );
   auto* p_second_alloc = my_allocator.allocate( alloc_size, alignof( std::vector<float> ) );

   my_allocator.free( p_second_alloc );
   p_second_alloc = nullptr;

   auto* p_third_alloc = my_allocator.allocate( alloc_size, alignof( bool[2] ) );
   return 0;
}
