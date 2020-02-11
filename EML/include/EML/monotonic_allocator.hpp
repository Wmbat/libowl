#pragma once

#include <EML/allocator_utils.hpp>

#include <memory>

namespace EML
{
   class monotonic_allocator final
   {
   public:
      monotonic_allocator( std::size_t size ) noexcept;

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept;

      template <class type_, class... args_>
      [[nodiscard]] type_* make_new( args_&&... args ) noexcept
      {
         if ( auto* p_alloc = allocate( sizeof( type_ ), alignof( type_ ) ) )
         {
            return new ( p_alloc ) type_( args... );
         }
         else
         {
            return nullptr;
         }
      }

      void clear( ) noexcept;

      std::size_t max_size( ) const noexcept;
      std::size_t memory_usage( ) const noexcept;
      std::size_t allocation_count( ) const noexcept;

   private:
      std::size_t total_size;
      std::size_t used_memory;
      std::size_t num_allocations;

      std::unique_ptr<std::byte[]> p_memory;
      std::byte* p_current_pos;
   };
} // namespace EML
