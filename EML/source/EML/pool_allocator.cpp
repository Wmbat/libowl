#include <EML/pool_allocator.hpp>

#include <utility>

namespace EML
{
   pool_allocator::pool_allocator( std::size_t const block_count, std::size_t const block_size ) noexcept :
      allocator_interface( block_count * block_size ), block_count( block_count ), block_size( block_size ),
      p_memory( std::make_unique<std::byte[]>( block_count * block_size * sizeof( header ) ) ), p_first_free( nullptr )
   {
      assert( block_count != 0 && "Cannot have no blocks in memory pool" );
      assert( block_size != 0 && "Cannot have a block size of zero" );

      p_first_free = reinterpret_cast<header*>( p_memory.get( ) );
      auto* p_base_cpy = p_first_free;

      for ( int i = 1; i < block_count; ++i )
      {
         std::size_t offset = i * ( block_size + sizeof( header ) );

         auto* p_new = reinterpret_cast<header*>( p_memory.get( ) + offset );
         p_base_cpy->p_next = p_new;
         p_base_cpy = p_new;
         p_base_cpy->p_next = nullptr;
      }
   }

   std::byte* pool_allocator::allocate( std::size_t size, std::size_t alignment ) noexcept 
   {
      assert( size != 0 && "Allocation size cannot be zero" );
      assert( size == 1024 && "Allocation size does not match pool block size" );

      if ( p_first_free )
      {
         std::byte* p_chunk_header = reinterpret_cast<std::byte*>( p_first_free );

         p_first_free = p_first_free->p_next;

         used_memory += block_size;
         ++num_allocations;

         return reinterpret_cast<std::byte*>( p_chunk_header + sizeof( header ) );
      }
      else
      {
         return nullptr;
      }
   }

   void pool_allocator::free( std::byte* p_location ) noexcept
   {
      assert( p_location != nullptr && "cannot free a nullptr" );

      auto* p_header = reinterpret_cast<header*>( p_location - sizeof( header ) );
      p_header->p_next = p_first_free;
      p_first_free = p_header;

      used_memory -= block_size;
      --num_allocations;
   }

   void pool_allocator::clear( ) noexcept
   {
      
   }
}
