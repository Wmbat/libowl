#pragma once

#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

namespace EML
{
   template<std::size_t block_sz_, std::size_t block_count_>
   class pool_allocator final : public allocator_interface
   {
   public:
      pool_allocator( ) = default;

   private:
      static constexpr std::size_t block_sz = block_sz_;
      static constexpr std::size_t block_count = block_count_;
      static constexpr std::size_t pool_sz = block_sz * block_count;
   };
} // namespace EML
