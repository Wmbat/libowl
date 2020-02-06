#include <EML/multipool_allocator.hpp>

#include <cmath>

namespace EML
{
   multipool_allocator::multipool_allocator(
      std::size_t block_count, std::size_t block_size, std::size_t pool_depth ) noexcept :
      allocator_interface( block_count * block_size * pool_depth ),
      block_count( block_count ), block_size( block_size ), pool_depth( pool_depth )
   {
      assert( block_count != 0 && "Cannot have no blocks in memory pool" );
      assert( block_size != 0 && "Cannot have a block size of zero" );

      std::size_t true_alloc_size = 0;
      for ( int i = 0; i <= pool_depth; ++i )
      {
         std::size_t const depth_pow = std::pow( 2, i );
         std::size_t const depth_block_count = block_count * depth_pow;
         std::size_t const depth_block_size = block_size / depth_pow;

         true_alloc_size += depth_block_count * ( sizeof( block_header ) + depth_block_size );
      }

      p_memory = std::make_unique<std::byte[]>( true_alloc_size + ( ( pool_depth + 1 ) * sizeof( block_header ) ) );

      p_depth_header = reinterpret_cast<block_header*>( p_memory.get( ) );
      for ( int i = 0; i <= pool_depth; ++i )
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
} // namespace EML
