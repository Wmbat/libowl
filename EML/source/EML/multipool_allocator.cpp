#include <EML/multipool_allocator.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

namespace EML
{
   multipool_allocator::multipool_allocator(
      std::size_t const block_count, std::size_t const block_size, std::size_t const pool_depth ) noexcept :
      block_count( block_count ),
      block_size( block_size ), pool_depth( pool_depth ), total_size( block_count * block_size * pool_depth ),
      used_memory( 0 ), num_allocations( 0 )
   {
      assert( block_count != 0 && "Cannot have no blocks in memory pool" );
      assert( block_size != 0 && "Cannot have a block size of zero" );
      assert( pool_depth != 0 && "Cannot have a pool depth of zero" );

      std::size_t true_alloc_size = 0;
      for ( int i = 0; i < pool_depth; ++i )
      {
         std::size_t const depth_pow = std::pow( 2, i );
         std::size_t const depth_block_count = block_count * depth_pow;
         std::size_t const depth_block_size = block_size / depth_pow;

         true_alloc_size += depth_block_count * ( sizeof( block_header ) + depth_block_size );
      }

      p_memory = std::make_unique<std::byte[]>( true_alloc_size + ( ( pool_depth + 1 ) * sizeof( block_header ) ) );

      p_depth_header = reinterpret_cast<block_header*>( p_memory.get( ) );
      for ( int i = 0; i < pool_depth; ++i )
      {
         std::size_t const depth_pow = std::pow( 2, i );
         std::size_t const depth_offset = ( pool_depth + 1 ) * sizeof( block_header );
         std::size_t const pool_offset = block_count * block_size * i;

         p_depth_header[i].p_next = reinterpret_cast<block_header*>( p_memory.get( ) + depth_offset + pool_offset );

         auto* p_base_cpy = p_depth_header[i].p_next;
         for ( int j = 1; j < block_count * depth_pow; ++j )
         {
            std::size_t const offset = j * ( block_size / depth_pow + sizeof( block_header ) );

            auto* p_new = reinterpret_cast<block_header*>( p_memory.get( ) + offset + depth_offset + pool_offset );
            p_base_cpy->p_next = p_new;
            p_base_cpy = p_new;
            p_base_cpy->p_next = nullptr;
         }
      }
   }

   multipool_allocator::pointer<std::byte> multipool_allocator::allocate(
      std::size_t size, std::size_t alignment ) noexcept
   {
      assert( size != 0 && "Allocation size cannot be zero" );
      assert( size <= block_size && "Allocation size cannot be greater than max pool size" );
      assert( alignment != 0 && "Allocation alignment cannot be zero" );

      auto const depth_index = std::clamp( block_size / size, std::size_t{1}, pool_depth ) - 1;

      if ( p_depth_header[depth_index].p_next )
      {
         std::byte* p_chunk_header = reinterpret_cast<std::byte*>( p_depth_header[depth_index].p_next );

         p_depth_header[depth_index].p_next = p_depth_header[depth_index].p_next->p_next;

         used_memory += block_size;
         ++num_allocations;

         return {reinterpret_cast<std::byte*>( p_chunk_header + sizeof( block_header ) ), depth_index};
      }
      else
      {
         return {nullptr, depth_index};
      }
   }

   void multipool_allocator::free( pointer<std::byte> alloc ) noexcept
   {
      assert( alloc.p_data != nullptr && "cannot free a nullptr" );
      assert( alloc.index >= 0 && "Cannot have a depth index below 1" );

      auto* p_header = reinterpret_cast<block_header*>( alloc.p_data - sizeof( block_header ) );
      p_header->p_next = p_depth_header[alloc.index].p_next;
      p_depth_header[alloc.index].p_next = p_header;

      used_memory -= block_size;
      --num_allocations;
   }

   void multipool_allocator::clear( ) noexcept
   {
      p_depth_header = reinterpret_cast<block_header*>( p_memory.get( ) );
      for ( int i = 0; i < pool_depth; ++i )
      {
         std::size_t const depth_pow = std::pow( 2, i );
         std::size_t const depth_offset = ( pool_depth + 1 ) * sizeof( block_header );
         std::size_t const pool_offset = block_count * block_size * i;

         p_depth_header[i].p_next = reinterpret_cast<block_header*>( p_memory.get( ) + depth_offset + pool_offset );

         auto* p_base_cpy = p_depth_header[i].p_next;
         for ( int j = 1; j < block_count * depth_pow; ++j )
         {
            std::size_t const offset = j * ( block_size / depth_pow + sizeof( block_header ) );

            auto* p_new = reinterpret_cast<block_header*>( p_memory.get( ) + offset + depth_offset + pool_offset );
            p_base_cpy->p_next = p_new;
            p_base_cpy = p_new;
            p_base_cpy->p_next = nullptr;
         }
      }

      used_memory = 0;
      num_allocations = 0;
   }

   std::size_t multipool_allocator::max_size( ) const noexcept { return total_size; }
   std::size_t multipool_allocator::memory_usage( ) const noexcept { return used_memory; }
   std::size_t multipool_allocator::allocation_count( ) const noexcept { return num_allocations; }
} // namespace EML
