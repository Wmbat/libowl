#include <EML/allocator_interface.hpp>
#include <EML/allocator_utils.hpp>

#include <memory>
#include <utility>

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

      struct allocation_header
      {
         std::size_t size;
         std::size_t adjustment;
      };

   public:
      free_list_allocator( std::size_t const size, policy placement_policy ) noexcept;

      [[nodiscard]] std::byte* allocate( std::size_t size, std::size_t alignment ) noexcept override;
      void free( std::byte* p_alloc ) noexcept override;

      void clear( ) noexcept override;

      template <class type_, class... args_>
      [[nodiscard]] uptr<type_> make_unique( args_&&... args ) noexcept
      {
         if ( auto* p_alloc = allocate( sizeof( type_ ), alignof( type_ ) ) )
         {
            return uptr<type_>( new ( p_alloc ) type_( args... ), [this]( type_* p_type ) {
               this->make_delete( p_type );
            } );
         }
         else
         {
            return uptr<type_>( nullptr, [this]( type_* p_type ) {
               this->make_delete( p_type );
            } );
         }
      }

   private:
      std::unique_ptr<std::byte[]> p_memory;
      block* p_free_blocks;

      policy placement_policy;
   };
} // namespace EML
