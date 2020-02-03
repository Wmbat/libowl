#include <EML/allocator_interface.hpp>

namespace EML
{
   allocator_interface::allocator_interface( std::size_t size ) noexcept : total_size( size ), used_memory( 0 ), num_allocations( 0 ) {}

   std::size_t allocator_interface::max_size( ) const noexcept { return total_size; }
   std::size_t allocator_interface::memory_usage( ) const noexcept { return used_memory; }
}
