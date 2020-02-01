#pragma once

#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <memory>

namespace EML
{
   class monotonic_allocator final : public allocator_interface
   {
   public:
      monotonic_allocator( std::size_t size ) noexcept;

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override;
      void free( std::byte* p_location ) noexcept override;

      void clear( ) noexcept override;

   private:
      std::unique_ptr<std::byte[]> p_memory;
      std::byte* p_current_pos;
   }; // class linear_allocator
} // namespace EML
