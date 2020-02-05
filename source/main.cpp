#include <ELL/logger.hpp>

#include <EML/pool_allocator.hpp>

int main( )
{
   EML::pool_allocator my_allocator{2, 1024};
   auto test = my_allocator.make_unique<int>(3);

   return 0;
}
