#pragma once

#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <cassert>
#include <cstdint>

namespace EML
{
   class pool_allocator final : public allocator_interface
   {
   private:
      struct header
      {
         header* p_next;
      };

   public:
      pool_allocator( std::size_t const block_count, std::size_t const block_size ) noexcept;
      pool_allocator( pool_allocator const& other ) noexcept = delete;
      pool_allocator( pool_allocator&& other ) noexcept;
      ~pool_allocator( ) noexcept;

      pool_allocator& operator=( pool_allocator const& rhs ) noexcept = delete;
      pool_allocator& operator=( pool_allocator&& rhs ) noexcept;

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override;
      void free( std::byte* p_location ) noexcept override;

   private:
      std::size_t block_count = 0;
      std::size_t block_size = 0;

      std::byte* p_memory = nullptr;
      header* p_first_free = nullptr;
   };
} // namespace EML
