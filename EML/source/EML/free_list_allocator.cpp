#include <EML/free_list_allocator.hpp>

#include <cassert>

namespace EML
{
   free_list_allocator::free_list_allocator( std::size_t size, policy placement_policy ) noexcept :
      allocator_interface( size ), p_memory( std::make_unique<std::byte[]>( size ) ), placement_policy( placement_policy )
   {
      assert( size > sizeof( block ) && "size of memory allocation is too small" );

      p_free_blocks = reinterpret_cast<block*>( p_memory.get( ) );
      p_free_blocks->size = size;
      p_free_blocks->p_next = nullptr;
   }

   std::byte* free_list_allocator::allocate(std::size_t size, std::size_t alignment) noexcept
   {
      assert( size != 0 && "allocation size cannot be zero." );
      assert( alignment != 0 && "memory alignment cannot be zero." );

      block* p_prev_free_block = nullptr;
   }

   std::size_t free_list_allocator::find_best_fit( std::size_t size, std::size_t alignment, block* p_prev_block, block* p_new_block ) const noexcept
   {
      block* p_block = p_free_blocks;
   }

   std::size_t free_list_allocator::find_first_fit( std::size_t size, std::size_t alignment, block* p_prev_block, block* p_new_block ) const noexcept
   {

   }
} // namespace EML
