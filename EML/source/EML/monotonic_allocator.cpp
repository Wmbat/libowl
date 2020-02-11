#include <EML/monotonic_allocator.hpp>

#include <cassert>

namespace EML
{
   monotonic_allocator::monotonic_allocator( std::size_t size ) noexcept :
      total_size( size ), num_allocations( 0 ), used_memory( 0 ), p_memory( std::make_unique<std::byte[]>( size ) ),
      p_current_pos( p_memory.get( ) )
   {}

   std::byte* monotonic_allocator::allocate( std::size_t size, std::size_t alignment ) noexcept
   {
      assert( size != 0 );

      auto const padding = get_forward_padding( reinterpret_cast<std::uintptr_t>( p_current_pos ), alignment );

      if ( padding + size + used_memory > total_size )
      {
         return nullptr;
      }

      std::byte* aligned_address = p_current_pos + padding;
      p_current_pos = aligned_address + size;

      used_memory += size + padding;
      ++num_allocations;

      return aligned_address;
   }

   void monotonic_allocator::clear( ) noexcept
   {
      p_current_pos = p_memory.get( );
      used_memory = 0;
      num_allocations = 0;
   }

   std::size_t monotonic_allocator::max_size( ) const noexcept { return total_size; }
   std::size_t monotonic_allocator::memory_usage( ) const noexcept { return used_memory; }
   std::size_t monotonic_allocator::allocation_count( ) const noexcept { return num_allocations; }
} // namespace EML
