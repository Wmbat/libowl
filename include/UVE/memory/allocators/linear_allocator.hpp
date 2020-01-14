#pragma once

#include <UVE/memory/allocators/allocator_interface.hpp>

namespace UVE
{
   class linear_allocator : public allocator_interface
   {
   public:
      linear_allocator( std::size_t size );
      virtual ~linear_allocator( );

      virtual std::byte* allocate( std::size_t size, std::size_t alignment ) override;
      virtual void free( std::byte* address ) override;

      void clear( ) noexcept;

   private:
      std::byte* p_start;
      std::byte* p_current_pos;
   }; // class linear_allocator
} // namespace UVE
