#pragma once

#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <cassert>
#include <utility>

namespace EML
{
   class monotonic_allocator final : public allocator_interface
   {
   public:
      monotonic_allocator( std::size_t size ) noexcept;
      monotonic_allocator( monotonic_allocator const& other ) noexcept = delete;
      monotonic_allocator( monotonic_allocator&& other ) noexcept;
      ~monotonic_allocator( ) noexcept;

      monotonic_allocator& operator=( monotonic_allocator const& rhs ) noexcept = delete;
      monotonic_allocator& operator=( monotonic_allocator&& rhs ) noexcept;

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override;
      void free( std::byte* p_location ) noexcept override;

      void clear( ) noexcept;

   private:
      std::size_t used_memory;
      std::size_t num_allocations;

      std::byte* p_start;
      std::byte* p_current_pos;
   }; // class linear_allocator
} // namespace EML
