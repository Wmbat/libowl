#pragma once

#include <EML/monotonic_allocator.hpp>

namespace EML
{
   monotonic_allocator::monotonic_allocator( std::size_t size ) noexcept :
      allocator_interface( size ), p_start( new std::byte[size] ), p_current_pos( p_start )
   {}
   monotonic_allocator::monotonic_allocator( monotonic_allocator&& other ) noexcept { *this = std::move( other ); }
   monotonic_allocator::~monotonic_allocator( ) noexcept
   {
      if ( p_start != nullptr )
      {
         delete[] p_start;
         p_start = nullptr;
      }

      p_current_pos = nullptr;
   }

   monotonic_allocator& monotonic_allocator::operator=( monotonic_allocator&& rhs ) noexcept
   {
      if ( this != &rhs )
      {
         used_memory = std::move( rhs.used_memory );

         num_allocations = std::move( rhs.num_allocations );

         p_start = rhs.p_start;
         rhs.p_start = nullptr;

         p_current_pos = rhs.p_current_pos;
         rhs.p_current_pos = nullptr;
      }

      return *this;
   }

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

   void monotonic_allocator::free( std::byte* p_location ) noexcept { assert( false && "Use clear() instead" ); }

   void monotonic_allocator::clear( ) noexcept
   {
      p_current_pos = p_start;
      used_memory = 0;
      num_allocations = 0;
   }
}
