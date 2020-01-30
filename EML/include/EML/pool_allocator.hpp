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
         auto* p_base_cpy = p_first_free;

         for( int i = 1; i < block_count; ++i )
         {
            std::size_t offset = i * ( block_size + sizeof( header ) );

            auto* p_new = reinterpret_cast<header*>( p_memory + offset );
            p_base_cpy->p_next = p_new;
            p_base_cpy = p_new;
            p_base_cpy->p_next = nullptr;
         }
      }

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override 
      {
         if ( p_first_free )
         {
            std::byte* p_first = reinterpret_cast<std::byte*>( p_first_free );
            const std::size_t offset = get_forward_padding( reinterpret_cast<std::uintptr_t>( p_first ), alignment, sizeof( header ) ); 
   
            p_first_free = p_first_free->p_next;

            used_memory += block_size;        
            ++num_allocations;

            return reinterpret_cast<std::byte*>( p_first_free + offset );
         }
         else
         {
            return nullptr;
         }
      }

      void free( std::byte* p_location ) noexcept override {}

   private:
      std::size_t const pool_size;
      std::size_t const block_count;
      std::size_t const block_size;

      std::byte* p_memory;
      header* p_first_free;
   };
} // namespace EML
