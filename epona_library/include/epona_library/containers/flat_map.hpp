/**
 * @file flat_map.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 6th, 2020.
 * @copyright MIT License.
 */

#pragma once

namespace ESL
{
   template <full_allocator allocator_>
   class hybrid_flat_map_base
   {
   public:
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = allocator_;
      using pointer = void*;

   protected:
      hybrid_flat_map_base( ) = delete;
      hybrid_flat_map_base( pointer p_first_element, size_type capacity, allocator_type* p_alloc ) :
         p_begin( p_first_element ), p_alloc( p_alloc ), cap( capacity )
      {}

   public:
      /**
       * @brief Return a pointer to the container's allocator.
       *
       * @return The pointer to the container's allocator
       */
      allocator_type* get_allocator( ) noexcept { return p_alloc; }
      /**
       * @brief Return a pointer to the container's allocator.
       *
       * @return The pointer to the container's allocator
       */
      allocator_type const* get_allocator( ) const noexcept { return p_alloc; }

      /**
       * @brief Check if the container has no element.
       *
       * @return True if the container is empty, otherwise false.
       */
      [[nodiscard]] constexpr bool empty( ) const noexcept { return count == 0; }
      /**
       * @brief Return the number of elements in the container.
       *
       * @return The size of the container.
       */
      size_type size( ) const noexcept { return count; }
      /**
       * @brief Return the maximum size of the container.
       *
       * @return The maximum value held by the #difference_type.
       */
      size_type max_size( ) const noexcept { return std::numeric_limits<difference_type>::max( ); }
      /**
       * @brief Return the container's capacity;
       *
       * @return The container's current memory capacity.
       */
      size_type capacity( ) const noexcept { return cap; }

   protected:
      pointer p_begin{ nullptr };
      allocator_type* p_alloc{ nullptr };
      size_type count{ 0 };
      size_type cap{ 0 };
   }
} // namespace ESL
