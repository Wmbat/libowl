#include <ELL/logger.hpp>

#include <EML/multipool_allocator.hpp>
#include <EML/monotonic_allocator.hpp>

int main( )
{
   EML::monotonic_allocator my_allocator{ 1024 };
   auto* p_test = my_allocator.make_new<int>( 10 );

   return 0;
}
