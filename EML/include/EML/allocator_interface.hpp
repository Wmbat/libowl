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
      std::size_t used_memory;
      std::size_t num_allocations;
   };
} // namespace EML
