#pragma once

#include <UVE/memory/allocators/allocator_interface.hpp>

namespace UVE
{
   class stack_allocator : public allocator_interface
   {
   public:
      stack_allocator( std::size_t size );

      virtual std::byte* allocate( std::size_t size, std::size_t allignment ) override;
      virtual void free( std::byte* p_address ) override;

   private:
      std::byte* beg;
      std::byte* end;
      std::byte* top;
   };
} // namespace UVE
