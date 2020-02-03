#pragma once

#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <memory>
#include <utility>

namespace EML
{
   class stack_allocator final : public allocator_interface
   {
   public:
      stack_allocator( std::size_t const size ) noexcept;

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override;
      void free( std::byte* p_address ) noexcept override;

      void clear( ) noexcept override;

   private:
      std::unique_ptr<std::byte[]> p_memory;
      std::byte* p_top;

   private:
      struct header
      {
         std::size_t adjustment;
      };
   };
} // namespace EML
