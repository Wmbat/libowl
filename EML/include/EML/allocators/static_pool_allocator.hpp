#pragma once

#include <EML/allocators/allocator_interface.hpp>

namespace EML
{
   template <std::size_t chunk_size_, std::size_t chunk_count_>
   class static_pool_allocator : public allocator_interface
   {
      static_assert( chunk_size_ > 0, "Size of a chunk cannot be 0" );
      static_assert( chunk_count_ > 0, "Cannot have 0 chunks" );

   public:

   private:
      static constexpr std::size_t CHUNK_SIZE = chunk_size_;
      static constexpr std::size_t CHUNK_COUNT = chunk_count_;
   };
} // namespace EML
