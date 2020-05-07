/**
 * @file flat_map.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 6th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <epona_library/containers/details.hpp>

namespace ESL
{
   template <full_allocator allocator_>
   class flat_map_base
   {
   public:
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = allocator_;
      using pointer = void*;

   protected:
      flat_map_base( ) = delete;
      flat_map_base( pointer p_first_element, size_type capacity, allocator_type* p_alloc ) :
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

   /**
    * @struct tiny_flat_map_align_and_size flat_map.hpp <ESL/containers/flat_map.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, April 29th, 2020
    * @copyright MIT License.
    * @brief The memory layout with padding of a tiny_flat_map
    *
    * @tparam any_ The type of objects that can be contained in the container.
    * @tparam allocator_ The type of the allocator used by the container.
    */
   template <class any_, complex_allocator<any_> allocator_>
   struct tiny_dynamic_array_align_and_size
   {
      std::aligned_storage_t<sizeof( flat_map_base<allocator_> ), alignof( flat_map_base<allocator_> )> base;
      std::aligned_storage_t<sizeof( std::size_t ), alignof( std::size_t )> padding;
      std::aligned_storage_t<sizeof( any_ ), alignof( any_ )> first_element;
   };

   template <class key_, class val_, complex_allocator<std::pair<key_, val_>> allocator_>
   class flat_avl_map_impl : public flat_map_base<allocator_>
   {
   };

   template <class key_, class val_, std::size_t buff_sz, complex_allocator<std::pair<key_, val_>> allocator_>
   class flat_avl_map :
      public flat_avl_map_impl<key_, val, allocator_>,
      details::static_array_storage<std::pair<key_, val_>, buff_sz>
   {
   };
} // namespace ESL
