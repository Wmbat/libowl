#include <EML/allocators/linear_allocator.hpp>

#include <cassert>

namespace EML
{
   linear_allocator::linear_allocator( std::size_t size )
   {
      total_size = size;
      p_start = new std::byte[total_size];
      p_current_pos = p_start;
   }

   linear_allocator::~linear_allocator( )
   {
      delete p_start;
      p_start = nullptr;
      p_current_pos = nullptr;
   }

   std::byte* linear_allocator::allocate( std::size_t size, std::size_t alignment )
   {
      assert( alignment >= sizeof( std::size_t ) );
      assert( size != 0 );

      auto const padding = get_forward_padding( reinterpret_cast<std::uintptr_t>( p_current_pos ), alignment );

      if ( padding + size > total_size - used_memory )
      {
         return nullptr;
      }

      std::byte* aligned_address = p_current_pos + padding;
      p_current_pos = aligned_address + size;

      used_memory += size + padding;
      ++num_allocations;

      return aligned_address;
   }

   void linear_allocator::free( std::byte* address ) { assert( false && "Use clear() instead" ); }

   void linear_allocator::clear( ) noexcept
   {
      p_current_pos = p_start;
      used_memory = 0;
      num_allocations = 0;
   }
} // namespace UVE
