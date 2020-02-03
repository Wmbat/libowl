#include <EML/free_list_allocator.hpp>

#include <cassert>

namespace EML
{
   free_list_allocator::free_list_allocator( std::size_t size, policy placement_policy ) noexcept :
      allocator_interface( size ), p_memory( std::make_unique<std::byte[]>( size ) ), placement_policy( placement_policy )
   {
      p_free_blocks = reinterpret_cast<block*>( p_memory.get( ) );
   }
} // namespace EML
