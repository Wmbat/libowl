
#include <ESL/allocators/allocator_utils.hpp>
#include <ESL/allocators/pool_allocator.hpp>
#include <ESL/containers/vector.hpp>

#include <ESL/utils/logger.hpp>

#include <vector>

struct moveable
{
   moveable( ) = default;
   explicit moveable( int i ) : i( i ) {}
   moveable( moveable const& other ) = delete;
   moveable( moveable&& other ) { i = std::move( other.i ); }

   moveable& operator=( moveable const& other ) = delete;
   moveable& operator=( moveable&& other )
   {
      i = std::move( other.i );
      return *this;
   }

   int i = 0;
};

struct copyable
{
   int i = 0;
};

void test( ESL::complex_allocator<int> auto& t )
{

}

int main( )
{
   std::size_t size = 4096 * 4;

   ESL::pool_allocator my_pool{ 1, size };
   ESL::vector<copyable, ESL::pool_allocator> my_vec{ &my_pool };

   test( my_pool );

   copyable a{ 20 };
   auto it = my_vec.insert( my_vec.cbegin( ), 5, a );

   return 0;
}
