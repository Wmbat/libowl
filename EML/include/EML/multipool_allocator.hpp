#pragma once

#include <EML/allocator_utils.hpp>

#include <memory>

namespace EML
{
   class multipool_allocator final
   {
   public:
      struct block_header
      {
         block_header* p_next;
      };

      template <class type_>
      struct pointer
      {
         type_* p_data;
         std::size_t const index;
      };

   public:
      multipool_allocator(
         std::size_t const block_count, std::size_t const block_size, std::size_t const pool_depth = 1 ) noexcept;

      pointer<std::byte> allocate( std::size_t size, std::size_t alignment ) noexcept;
      void free( pointer<std::byte> alloc ) noexcept;

      template <class type_, class... args_>
      [[nodiscard]] pointer<type_> make_new( args_&&... args ) noexcept
      {
         auto const alloc = allocate( sizeof( type_ ), alignof( type_ ) );
         if ( alloc.p_data )
         {
            return {new ( alloc.p_data ) type_( args... ), alloc.index};
         }
         else
         {
            return {nullptr, 0};
         }
      }

      template <class type_>
      void make_delete( pointer<type_> type ) noexcept
      {
         if ( type.p_data )
         {
            type.p_data->~type_( );
            free( type );
         }
      }

      void clear( ) noexcept;

      std::size_t max_size( ) const noexcept;
      std::size_t memory_usage( ) const noexcept;
      std::size_t allocation_count( ) const noexcept;

   private:
      std::size_t block_count;
      std::size_t block_size;
      std::size_t pool_depth;

      std::size_t total_size;
      std::size_t used_memory;
      std::size_t num_allocations;

      std::unique_ptr<std::byte[]> p_memory;
      block_header* p_depth_header;
   };
} // namespace EML
