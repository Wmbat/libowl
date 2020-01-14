#include <UVE/memory/allocators/stack_allocator.hpp>

namespace UVE
{
   stack_allocator::stack_allocator( std::size_t size )
   {
      beg = new std::byte[size];
      end = beg + size;
      top = beg;
   }

   std::byte* allocate( std::size_t size, std::size_t allignment ) {}

   void free( std::byte* p_address ) {}
} // namespace UVE
