#pragma once

#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <cstdint>

namespace EML
{
   class pool_allocator final : public allocator_interface
   {
   private:
      struct header
      {
         header* p_next;
      };

   public:
      pool_allocator( std::size_t const block_count, std::size_t const block_size ) :
         block_count( block_count ), block_size( block_size ), pool_size( block_size * block_count ),
         p_memory( new std::byte[block_count * block_size * sizeof( header )] ), p_first_free( nullptr )
      {
         p_first_free = reinterpret_cast<header*>( p_memory ); 
      }

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override {}

      void free( std::byte* p_location ) noexcept override {}

   private:
      std::size_t pool_size;
      std::size_t block_count;
      std::size_t block_size;

      std::byte* p_memory;
      header* p_first_free;
   };
} // namespace EML
