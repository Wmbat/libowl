#pragma once

#include <EML/allocators/allocator_interface.hpp>

#include <cassert>
#include <utility>

namespace EML
{
   template <std::size_t size_>
   class linear_allocator : public allocator_interface
   {
      static_assert( size_ > 0, "Size of allocator cannot be 0" );

   public:
      linear_allocator( ) : p_start( new std::byte[MAX_SIZE] ), p_current_pos( p_start )
      {
         used_memory = 0;
         num_allocations = 0;
      }
      linear_allocator( linear_allocator const& other ) = delete;
      linear_allocator( linear_allocator&& other ) { *this = std::move( other ); }
      ~linear_allocator( )
      {
         if ( p_start )
         {
            delete[] p_start;
            p_start = nullptr;
         }

         p_current_pos = nullptr;
      }

      linear_allocator& operator=( linear_allocator const& rhs ) = delete;
      linear_allocator& operator=( linear_allocator&& rhs )
      {
         if ( this != rhs )
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

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) override
      {
         assert( size != 0 );

         auto const padding = get_forward_padding( reinterpret_cast<std::uintptr_t>( p_current_pos ), alignment );

         if ( padding + size + used_memory > MAX_SIZE )
         {
            return nullptr;
         }

         std::byte* aligned_address = p_current_pos + padding;
         p_current_pos = aligned_address + size;

         used_memory += size + padding;
         ++num_allocations;

         return aligned_address;
      }
      void free( std::byte* address ) override { assert( false && "Use clear() instead" ); }

      void clear( ) noexcept
      {
         p_current_pos = p_start;
         used_memory = 0;
         num_allocations = 0;
      }

   private:
      static constexpr std::size_t MAX_SIZE = size_;

      std::byte* p_start;
      std::byte* p_current_pos;
   }; // class linear_allocator
} // namespace EML
