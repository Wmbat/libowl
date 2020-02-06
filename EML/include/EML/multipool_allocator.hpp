#pragma once

#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <memory>

namespace EML
{
   class multipool_allocator : public allocator_interface
   {
   public:
      struct block_header
      {
         block_header* p_next;
      };

   public:
      multipool_allocator( std::size_t block_count, std::size_t block_size, std::size_t pool_depth = 0 ) noexcept;

      std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override;
      void free( std::byte* p_alloc ) noexcept override;

      void clear( ) noexcept override;

   private:
      std::size_t block_count;
      std::size_t block_size;
      std::size_t pool_depth;

      std::unique_ptr<std::byte[]> p_memory;
      block_header* p_depth_header;
   };
} // namespace EML
