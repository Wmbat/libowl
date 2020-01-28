#pragma once

#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <cassert>
#include <utility>

namespace EML
{
   template <std::size_t size_>
   class stack_allocator final : public allocator_interface
   {
      static_assert( size_ > 0, "Size of allocator cannot be 0" );

   private:
      struct header
      {
         std::size_t adjustment;
      };

   public:
      stack_allocator( ) noexcept
      {
         p_start = new std::byte[MAX_SIZE];
         p_top = p_start;

         used_memory = 0;
         num_allocations = 0;
      }
      stack_allocator( stack_allocator const& other ) noexcept = delete;
      stack_allocator( stack_allocator&& other ) noexcept { *this = std::move( other ); }
      ~stack_allocator( ) noexcept
      {
         if ( p_start )
         {
            delete[] p_start;
            p_start = nullptr;
         }

         p_top = nullptr;
      }

      stack_allocator& operator=( stack_allocator const& rhs ) noexcept = delete;
      stack_allocator& operator=( stack_allocator&& rhs ) noexcept
      {
         if ( this != &rhs )
         {
            used_memory = std::move( rhs.used_memory );
            num_allocations = std::move( rhs.num_allocations );

            p_start = rhs.p_start;
            rhs.p_start = nullptr;

            p_top = rhs.p_top;
            rhs.p_top = nullptr;
         }

         return *this;
      }

      std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override
      {
         assert( size != 0 );

         auto const padding = get_forward_padding( reinterpret_cast<std::uintptr_t>( p_top ), alignment, sizeof( header ) );

         if ( padding + size + used_memory > MAX_SIZE )
         {
            return nullptr;
         }

         std::byte* aligned_address = p_top + padding;

         auto* header_address = reinterpret_cast<header*>( aligned_address - sizeof( header ) );
         header_address->adjustment = padding;

         p_top = aligned_address + size;

         used_memory += size + padding;
         ++num_allocations;

         return aligned_address;
      }
      void free( std::byte* p_address ) noexcept override
      {
         if ( p_address )
         {
            auto* header_address = reinterpret_cast<header*>( p_address - sizeof( header ) );
            used_memory -= p_top - p_address + header_address->adjustment;

            p_top = p_address - header_address->adjustment;

            --num_allocations;
         }
      }

   private:
      static constexpr std::size_t MAX_SIZE = size_;

      std::byte* p_start;
      std::byte* p_top;
   };
} // namespace EML
