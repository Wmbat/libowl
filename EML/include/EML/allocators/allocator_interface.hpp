#pragma once

#include <cstddef>
#include <cstdint>

namespace EML
{
   class allocator_interface
   {
   protected:
      allocator_interface( ) = default;
      virtual ~allocator_interface( ) = default;

   public:
      virtual std::byte* allocate( std::size_t size, std::size_t allignment = sizeof( std::size_t ) ) = 0;
      virtual void free( std::byte* p_location ) = 0;

      template <class type_, class... args_>
      [[nodiscard]] type_* make_new( args_&&... args )
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

      template <class type_>
      void make_delete( type_* p_type )
      {
         if ( p_type )
         {
            p_type->~type_( );
            free( reinterpret_cast<std::byte*>( p_type ) );
         }
      }

   protected:
      constexpr std::size_t get_backward_padding( std::uintptr_t address, std::size_t aligment ) const noexcept
      {
         auto const padding = address & ( aligment - 1 );

         return padding == aligment ? 0 : padding;
      }
      constexpr std::size_t get_forward_padding( std::uintptr_t address, std::size_t alignment ) const noexcept
      {
         auto const padding = alignment - ( address & ( alignment - 1 ) );

         return padding == alignment ? 0 : padding;
      }

      constexpr std::size_t get_forward_padding( std::uintptr_t address, std::size_t alignment, std::size_t header_size ) const noexcept
      {
         auto padding = get_forward_padding( address, alignment );

         if ( padding < header_size )
         {
            auto const needed_space = header_size - padding;

            padding += alignment * ( needed_space / alignment );
            if ( needed_space % alignment > 0 )
            {
               padding += alignment;
            }
         }

         return padding;
      }

   protected:
      std::size_t used_memory;
      std::size_t num_allocations;
   };
} // namespace EML
