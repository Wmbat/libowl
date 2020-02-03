#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <utility>
#include <memory>

namespace EML
{
   class free_list_allocator final : public allocator_interface
   {
   private:
      enum class policy
      {
         e_first_fit,
         e_best_fit
      };

      struct block
      {
         std::size_t size;
         block* p_next;
      };

   public:
      free_list_allocator( std::size_t const size, policy placement_policy ) noexcept;

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override;
      void free( std::byte* p_alloc ) noexcept override;

      void clear( ) noexcept override;

   private:
      std::unique_ptr<std::byte[]> p_memory;
      block* p_free_blocks;

      policy placement_policy;

   private:
      struct allocation_header
      {
         std::size_t size;
         std::size_t adjustment;
      };
   };
}
