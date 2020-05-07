/**
 * @file dynamic_array.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, April 23rd, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <epona_library/containers/details.hpp>
#include <epona_library/allocators/allocator_utils.hpp>
#include <epona_library/allocators/multipool_allocator.hpp>
#include <epona_library/utils/compare.hpp>
#include <epona_library/utils/concepts.hpp>
#include <epona_library/utils/error_handling.hpp>
#include <epona_library/utils/iterators/random_access_iterator.hpp>

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>

namespace ESL
{
   /**
    * @class dynamic_array_base dynamic_array.hpp <ESL/containers/dynamic_array.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, April 22th, 2020
    * @copyright MIT License.
    * @brief A common base for all dynamic arrays using custom allocators.
    *
    * @tparam allocator_ The type of the allocator used by the container.
    */
   template <full_allocator allocator_>
   class dynamic_array_base
   {
   public:
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = allocator_;
      using pointer = void*;

   protected:
      dynamic_array_base( ) = delete;
      /**
       * @brief Set the default data of the container.
       *
       * @param[in] p_first_element A pointer to the first element in the containen.
       * @param[in] capacity The starting capacity of the container.
       * @param[in] p_alloc A pointer to the allocator used by the container.
       */
      dynamic_array_base( pointer p_first_element, size_type capacity, allocator_type* p_alloc ) :
         p_begin( p_first_element ), p_alloc( p_alloc ), cap( capacity )
      {}

      /**
       * @brief A special function to grow the container's memory with trivial types.
       *
       * @param[in] p_first_element A pointer to the first element in the container.
       * @param[in] min_cap The maximum capacity to grow the container by.
       * @param[in] type_size The size of the trivial type.
       * @param[in] type_align The alignment of the trivial type.
       */
      void grow_trivial( void* p_first_element, size_type min_cap, size_type type_size, size_type type_align )
      {
         size_type const new_capacity =
            std::clamp( 2 * capacity( ) + 1, min_cap, std::numeric_limits<size_type>::max( ) );

         if ( p_begin == p_first_element )
         {
            void* p_new = p_alloc->allocate( new_capacity * type_size, type_align );
            if ( !p_new )
            {
               throw std::bad_alloc{ };
            }

            memcpy( p_new, p_begin, size( ) * type_size );

            p_begin = p_new;
         }
         else
         {
            void* p_new = p_alloc->reallocate( p_begin, new_capacity * type_size );
            if ( !p_new )
            {
               throw std::bad_alloc{ };
            }

            p_begin = p_new;
         }

         cap = new_capacity;
      }

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
      void* p_begin{ nullptr };
      allocator_type* p_alloc{ nullptr };
      size_type count{ 0 };
      size_type cap{ 0 };
   };

   /**
    * @struct tiny_dynamic_array_align_and_size dynamic_array.hpp <ESL/containers/dynamic_array.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, April 29th, 2020
    * @copyright MIT License.
    * @brief The memory layout with padding of a tiny_dynamic_array
    *
    * @tparam any_, The type of objects that can be contained in the container.
    * @tparam allocator_ The type of the allocator used by the container.
    */
   template <class any_, complex_allocator<any_> allocator_>
   struct tiny_dynamic_array_align_and_size
   {
      std::aligned_storage_t<sizeof( dynamic_array_base<allocator_> ), alignof( dynamic_array_base<allocator_> )> base;
      std::aligned_storage_t<sizeof( std::size_t ), alignof( std::size_t )> padding;
      std::aligned_storage_t<sizeof( any_ ), alignof( any_ )> first_element;
   };

   /**
    * @class tiny_dynamic_array_impl dynamic_array.hpp <ESL/containers/dynamic_array.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, April 29th, 2020
    * @copyright MIT License.
    * @brief The implementation of the common functions that all dynamic_arrays should use.
    *
    * @tparam any_, The type of objects that can be contained in the container.
    * @tparam allocator_ The type of the allocator used by the container.
    */
   template <class any_, complex_allocator<any_> allocator_>
   class tiny_dynamic_array_impl : public dynamic_array_base<allocator_>
   {
      using super = dynamic_array_base<allocator_>;

   public:
      using value_type = any_;
      using allocator_type = typename super::allocator_type;
      using reference = value_type&;
      using const_reference = value_type const&;
      using pointer = value_type*;
      using const_pointer = value_type const*;
      using size_type = typename super::size_type;
      using difference_type = typename super::difference_type;
      using iterator = random_access_iterator<value_type>;
      using const_iterator = random_access_iterator<value_type const>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   protected:
      tiny_dynamic_array_impl( ) = delete;
      /**
       * @brief Sets the capacity and the allocator of the container.
       *
       * @param[in] capacity The default capacity of the container.
       * @param[in] p_alloc The allocator from which memory will be fetched.
       */
      explicit tiny_dynamic_array_impl( size_type capacity, allocator_type* p_alloc ) :
         super( get_first_element( ), capacity, p_alloc )
      {}

      /**
       * @brief Check if the container is currently using the static memory buffer.
       *
       * @return True if the container is using the static memory buffer, otherwise false.
       */
      bool is_static( ) const noexcept { return super::p_begin == get_first_element( ); }

   public:
      /**
       * @brief Clear the container's data and surrenders the memory allocation.
       *
       * @details Clear the container's data and surrenders the memory allocation only if the dynamic_array is not using the
       * static memory storage.
       */
      virtual ~tiny_dynamic_array_impl( )
      {
         if ( !is_static( ) )
         {
            clear( );

            if ( super::p_alloc && super::p_begin )
            {
               super::p_alloc->deallocate( super::p_begin );
            }
         }
      }

      /**
       * @brief Replaces the content of the container with that of another.
       *
       * @param[in] rhs The container to use as data source.
       *
       * @return A reference to the current container.
       */
      tiny_dynamic_array_impl& operator=( tiny_dynamic_array_impl const& rhs )
      {
         if ( this == &rhs )
         {
            return *this;
         }

         if ( !super::p_alloc )
         {
            super::p_alloc = rhs.p_alloc;
         }

         size_type const other_sz = rhs.size( );
         size_type curr_sz = super::size( );

         if ( curr_sz >= other_sz )
         {
            iterator new_end = begin( );
            if ( other_sz )
            {
               new_end = std::copy( rhs.begin( ), rhs.end( ), new_end );
            }

            destroy_range( new_end, end( ) );

            super::count = other_sz;

            return *this;
         }
         else
         {
            if ( other_sz > super::capacity( ) )
            {
               destroy_range( begin( ), end( ) );
               super::count = curr_sz = 0;

               grow( other_sz );
            }
            else
            {
               std::copy( rhs.begin( ), rhs.begin( ) + curr_sz, begin( ) );
            }

            std::uninitialized_copy( rhs.begin( ) + curr_sz, rhs.end( ), begin( ) + curr_sz );

            super::count = other_sz;

            return *this;
         }
      }

      /**
       * @brief Replaces the content of the container with that of another.
       *
       * @param[in] rhs The container to use as data source.
       *
       * @return A reference to the current container.
       */
      tiny_dynamic_array_impl& operator=( tiny_dynamic_array_impl&& rhs )
      {
         if ( this == &rhs )
         {
            return *this;
         }

         if ( !super::p_alloc )
         {
            super::p_alloc = rhs.p_alloc;
            rhs.p_alloc = nullptr;
         }

         // If not static, steal buffer.
         if ( !rhs.is_static( ) )
         {
            destroy_range( begin( ), end( ) );
            if ( !is_static( ) && super::p_begin )
            {
               super::p_alloc->deallocate( super::p_begin );
            }

            super::count = rhs.count;
            rhs.count = 0;

            super::cap = rhs.cap;
            rhs.cap = 0;

            super::p_begin = rhs.p_begin;
            rhs.p_begin = nullptr;

            rhs.p_alloc = nullptr;

            return *this;
         }

         size_type const other_sz = rhs.size( );
         size_type curr_sz = super::size( );

         // if we have enough space, move the data from static buffer
         // and delete the leftover data we have.
         if ( curr_sz >= other_sz )
         {
            iterator new_end = begin( );
            if ( other_sz )
            {
               new_end = std::move( rhs.begin( ), rhs.end( ), new_end );
            }

            destroy_range( new_end, end( ) );

            super::count = other_sz;

            rhs.clear( );
            rhs.p_alloc = nullptr;

            return *this;
         }
         else // resize and move data from static buffer.
         {
            if ( other_sz > super::capacity( ) )
            {
               destroy_range( begin( ), end( ) );
               super::count = curr_sz = 0;

               grow( other_sz );
            }
            else if ( curr_sz )
            {
               std::move( rhs.begin( ), rhs.begin( ) + curr_sz, begin( ) );
            }

            std::uninitialized_move( rhs.begin( ) + curr_sz, rhs.end( ), begin( ) + curr_sz );

            super::count = other_sz;

            rhs.clear( );
            rhs.p_alloc = nullptr;

            return *this;
         }
      }

      /**
       * @brief Compare the contents of two containers.
       *
       * @details Checks if the contents of the current container and rhs are equal, that is, they have the same number
       * of elements and each element in lhs compares equal with the element in rhs at the same position. #value_type
       * must meet the <a
       * href="https://en.cppreference.com/w/cpp/concepts/equality_comparable">std::equality_comparable</a> to use this
       * function.
       *
       * @tparam  other_   The allocator of the other dynamic_array.
       * @param   rhs      The dynamic_array to compare against.
       *
       * @return True if the two dynamic_arrays have the same data, otherwise false
       */
      template <complex_allocator<value_type> other_ = allocator_>
      constexpr bool operator==( tiny_dynamic_array_impl<value_type, other_> const& rhs ) const
         requires std::equality_comparable<value_type>
      {
         return std::equal( cbegin( ), cend( ), rhs.cbegin( ), rhs.cend( ) );
      }

      /**
       * @brief Perform a lexicographical compare on the elements of the two dynamic_arrays.
       *
       * @tparam  other_   The allocator of the other dynamic_array.
       * @param   rhs      The dynamic_array to compare against.
       *
       * @return An ordering defining the relationship between the two dynamic_arrays.
       */
      template <complex_allocator<value_type> other_ = allocator_>
      constexpr auto operator<=>( tiny_dynamic_array_impl<value_type, other_> const& rhs )
      {
         return std::lexicographical_compare_three_way(
            cbegin( ), cend( ), rhs.cbegin( ), rhs.cend( ), synth_three_way );
      }

      /**
       * @brief Replaces the content of the container with count copies of value value.
       *
       * @details Removes all elements currently present in the container and places count copies of value value.All
       * iterators, pointers and references to the elements of the container are invalidated. The past-the-end iterator
       * is also invalidated. To call this function, #value_type must satisfy the <a
       * href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a>
       *
       * @param[in] count The new size of the container.
       * @param[in] value The value to initialize elements from the container with.
       */
      void assign( size_type count, const_reference value ) requires std::copyable<value_type>
      {
         clear( );

         if ( count > super::capacity( ) )
         {
            grow( count );
         }

         super::count += count;

         std::uninitialized_fill( begin( ), end( ), value );
      }

      /**
       * @brief Replaces the contents of the container with copies of those in the range [first, last).
       *
       * @details Removes all elements currently present in the container and places copies of the
       * range [first, last}. All iterators, pointers and references to the elements of the container are invalidated.
       * The past-the-end iterator is also invalidated. To call this function, #value_type must satisfy the <a
       * href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a>
       *
       * @tparam it_ The type of the iterators. Must comply with the std::input_iterator requirements.
       *
       * @param[in] first The first element to assign inclusive.
       * @param[in] last The last element to assign exclusive.
       */
      template <std::input_iterator it_>
      void assign( it_ first, it_ last ) requires std::copyable<value_type>
      {
         clear( );

         size_type const count = std::distance( first, last );
         if ( count > super::capacity( ) )
         {
            grow( count );
         }

         super::count += count;

         std::uninitialized_copy( first, last, begin( ) );
      }

      /**
       * @brief Replaces the contents of the container with the elements from the <a
       * href="https://en.cppreference.com/w/cpp/utility/initializer_list">std::initializer_list</a> initializer_list.
       *
       * @details Removes all elements currently present in the container and places copies of the elements contained
       * within the <a href="https://en.cppreference.com/w/cpp/utility/initializer_list">std::initializer_list</a>
       * initializer_list. All iterators, pointers and references to the elements of the container are invalidated.
       * The past-the-end iterator is also invalidated. To call this function, #value_type must satisfy the <a
       * href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a>
       *
       * @param[in] initializer_list The initializer list to copy the values from.
       */
      void assign( std::initializer_list<value_type> initializer_list ) requires std::copyable<value_type>
      {
         assign( initializer_list.begin( ), initializer_list.end( ) );
      }

      /**
       * @brief Return a #reference to the element at the index position in the container.
       *
       * @details Return a #reference to the element at the index position in the container. Performs
       * bounds checking. Will not throw if ESL_NO_EXCEPTIONS is defined.
       *
       * @throw std::out_of_range if index >= size().
       *
       * @param[in] index The index position of the desired element.
       *
       * @return A #reference to the requested element.
       */
      reference at( size_type index )
      {
         if ( index >= super::count )
         {
            throw std::out_of_range{ "Index: " + std::to_string( index ) + " is out of bounds" };
         }
         else
         {
            return super::p_alloc[index];
         }
      }
      /**
       * @brief Return a #const_reference to the element at the index position in the container.
       *
       * @details Return a #const_reference to the element at the index position in the container. Performs
       * bounds checking. Will not throw if ESL_NO_EXCEPTIONS is defined.
       *
       * @throw std::out_of_range if index >= size().
       *
       * @param[in] index The index position of the desired element.
       *
       * @return A #const_reference to the requested element.
       */
      const_reference at( size_type index ) const
      {
         if ( index >= super::count )
         {
            throw std::out_of_range{ "Index: " + std::to_string( index ) + " is out of bounds" };
         }
         else
         {
            return super::p_alloc[index];
         }
      }

      /**
       * @brief Return a #reference to the element at the index position in the container.
       *
       * @details Return a #reference to the element at the index position in the container. Does not perform any
       * bounds checking.
       *
       * @param[in] index The index position of the desired element.
       *
       * @return A #reference to the requested element.
       */
      reference operator[]( size_type index ) noexcept
      {
         assert( index < super::count && "Index out of bounds." );

         return static_cast<pointer>( super::p_begin )[index];
      }
      /**
       * @brief Return a #const_reference to the element at the index position in the container.
       *
       * @details Return a #const_reference to the element at the index position in the container. Does not perform any
       * bounds checking.
       *
       * @param[in] index The index position of the desired element.
       *
       * @return A #const_reference to the requested element.
       */
      const_reference operator[]( size_type index ) const noexcept
      {
         assert( index < super::count && "Index out of bounds." );

         return static_cast<pointer>( super::p_begin )[index];
      }

      /**
       * @brief Return a #reference to the first element in the container.
       *
       * @return A #reference to the first element in the container.
       */
      reference front( ) noexcept
      {
         assert( !super::empty( ) && "No elements in the container" );
         return *begin( );
      }
      /**
       * @brief Return a #const_reference to the first element in the container.
       *
       * @return A #const_reference to the first element in the container.
       */
      const_reference front( ) const noexcept
      {
         assert( !super::empty( ) && "No elements in the container" );
         return *cbegin( );
      }

      /**
       * @brief Return a #reference to the last element in the container.
       *
       * @return A #reference to the last element in the container.
       */
      reference back( ) noexcept
      {
         assert( !super::empty( ) && "No elements in the container" );
         return *( end( ) - 1 );
      }
      /**
       * @brief Return a #const_reference to the last element in the container.
       *
       * @return A #const_reference to the last element in the container.
       */
      const_reference back( ) const noexcept
      {
         assert( !super::empty( ) && "No elements in the container" );
         return *( cend( ) - 1 );
      }

      /**
       * @brief Return a #pointer to the container's memory.
       *
       * @return A #pointer to the container's memory
       */
      pointer data( ) noexcept { return pointer{ &( *begin( ) ) }; }
      /**
       * @brief Return a #const_pointer to the container's memory.
       *
       * @return A #const_pointer to the container's memory
       */
      const_pointer data( ) const noexcept { return const_pointer{ &( *cbegin( ) ) }; }

      iterator begin( ) noexcept { return iterator{ static_cast<pointer>( super::p_begin ) }; }
      const_iterator begin( ) const noexcept { return const_iterator{ static_cast<pointer>( super::p_begin ) }; }
      const_iterator cbegin( ) const noexcept { return const_iterator{ static_cast<pointer>( super::p_begin ) }; }

      iterator end( ) noexcept { return iterator{ begin( ) + super::count }; }
      const_iterator end( ) const noexcept { return const_iterator{ begin( ) + super::count }; }
      const_iterator cend( ) const noexcept { return const_iterator{ cbegin( ) + super::count }; }

      reverse_iterator rbegin( ) noexcept { return reverse_iterator{ end( ) }; }
      const_reverse_iterator rbegin( ) const noexcept { return const_reverse_iterator{ cend( ) }; }
      const_reverse_iterator rcbegin( ) const noexcept { return const_reverse_iterator{ cend( ) }; }

      reverse_iterator rend( ) noexcept { return reverse_iterator{ begin( ) }; }
      const_reverse_iterator rend( ) const noexcept { return const_reverse_iterator{ cbegin( ) }; }
      const_reverse_iterator rcend( ) const noexcept { return const_reverse_iterator{ cbegin( ) }; }

      /**
       * @brief Increase the capacity of the container to a value greater than or equal to new_cap.
       *
       * @details Increase the capacity of the dynamic_array to a value that's greater or equal to new_cap. If new_cap is
       * greater than the current capacity(), new storage is allocated, otherwise the method does nothing. reserve()
       * does not change the size of the dynamic_array. If new_cap is greater than capacity(), all iterators, including the
       * past-the-end iterator, and all references to the elements are invalidated. Otherwise, no iterators or
       * references are invalidated.
       *
       * @param[in] new_cap The new capacity of the dynamic_array.
       */
      void reserve( size_type new_cap )
      {
         if ( new_cap > super::capacity( ) )
         {
            grow( new_cap );
         }
      }

      /**
       * @brief Erases all elements from the container
       *
       * @details Erases all elements from the container. After this call, size() return zero. Invalidates any
       * references, pointers, or iterators referring to contained elements. Any past-the-end iterators are also
       * invalidated. Leaves the capacity() of the dynamic_array unchanged
       */
      void clear( ) noexcept
      {
         destroy_range( begin( ), end( ) );
         super::count = 0;
      }

      iterator insert( const_iterator pos, const_reference value ) requires std::copyable<value_type>
      {
         if ( pos == cend( ) )
         {
            push_back( value );

            return end( ) - 1;
         }

         assert( pos >= cbegin( ) && "Insertion iterator is out of bounds" );
         assert( pos <= cend( ) && "Insertion iterator is past the end" );

         iterator new_pos;
         if ( super::size( ) >= super::capacity( ) )
         {
            size_type offset = pos - cbegin( );
            grow( );
            new_pos = begin( ) + offset;
         }
         else
         {
            new_pos = begin( ) + ( pos - cbegin( ) );
         }

         new ( static_cast<void*>( &( *end( ) ) ) ) value_type( std::move( back( ) ) );

         std::move_backward( new_pos, end( ) - 1, end( ) );

         ++super::count;

         const_pointer p_element = &value;
         if ( pointer{ &( *new_pos ) } <= p_element && pointer{ &( *end( ) ) } > p_element )
         {
            ++p_element;
         }

         *new_pos = *p_element;

         return new_pos;
      }

      iterator insert( const_iterator pos, value_type&& value ) requires std::movable<value_type>
      {
         if ( pos == cend( ) )
         {
            push_back( std::move( value ) );

            return end( ) - 1;
         }

         assert( pos >= cbegin( ) && "Insertion iterator is out of bounds" );
         assert( pos <= cend( ) && "Insertion iterator is past the end" );

         iterator new_pos;
         if ( super::size( ) >= super::capacity( ) )
         {
            size_type offset = pos - cbegin( );
            grow( );
            new_pos = begin( ) + offset;
         }
         else
         {
            new_pos = begin( ) + ( pos - cbegin( ) );
         }

         new ( static_cast<void*>( &( *end( ) ) ) ) value_type( std::move( back( ) ) );

         std::move_backward( new_pos, end( ) - 1, end( ) );

         ++super::count;

         pointer p_element = &value;
         if ( pointer{ &( *new_pos ) } <= p_element && pointer{ &( *end( ) ) } > p_element )
         {
            ++p_element;
         }

         *new_pos = std::move( *p_element );

         return new_pos;
      }

      iterator insert( const_iterator pos, size_type count, const_reference value ) requires std::copyable<value_type>
      {
         size_type start_index = pos - cbegin( );

         if ( pos == cend( ) )
         {
            if ( super::size( ) + count >= super::capacity( ) )
            {
               grow( super::size( ) + count );
            }

            std::uninitialized_fill_n( end( ), count, value );

            super::count += count;

            return begin( ) + start_index;
         }

         assert( pos >= cbegin( ) && "Insertion iterator is out of bounds" );
         assert( pos <= cend( ) && "Insertion iterator is past the end" );

         reserve( super::size( ) + count );

         iterator updated_pos = begin( ) + start_index;

         if ( iterator old_end = end( ); end( ) - updated_pos >= count )
         {
            std::uninitialized_move( end( ) - count, end( ), end( ) );

            super::count += count;

            std::move_backward( updated_pos, old_end - count, old_end );
            std::fill_n( updated_pos, count, value );
         }
         else
         {
            size_type move_count = old_end - updated_pos;
            super::count += count;

            std::uninitialized_move( updated_pos, old_end, end( ) - move_count );
            std::fill_n( updated_pos, move_count, value );
            std::uninitialized_fill_n( old_end, count - move_count, value );
         }

         return updated_pos;
      }

      template <std::input_iterator it_>
      iterator insert( const_iterator pos, it_ first, it_ last ) requires std::copyable<value_type>
      {
         size_type start_index = pos - cbegin( );
         size_type count = std::distance( first, last );

         if ( pos == cend( ) )
         {
            if ( super::size( ) + count >= super::capacity( ) )
            {
               grow( super::size( ) + count );
            }

            std::uninitialized_copy( first, last, end( ) );

            super::count += count;

            return begin( ) + start_index;
         }

         assert( pos >= cbegin( ) && "Insertion iterator is out of bounds" );
         assert( pos <= cend( ) && "Insertion iterator is past the end" );

         reserve( super::size( ) + count );

         iterator updated_pos = begin( ) + start_index;

         if ( iterator old_end = end( ); end( ) - updated_pos >= count )
         {
            std::uninitialized_move( end( ) - count, end( ), end( ) );

            super::count += count;

            std::move_backward( updated_pos, old_end - count, old_end );
            std::copy( first, last, updated_pos );
         }
         else
         {
            size_type move_count = old_end - updated_pos;
            super::count += count;

            std::uninitialized_move( updated_pos, old_end, end( ) - move_count );

            for ( auto it = updated_pos; count > 0; --count )
            {
               *it = *first;

               ++it;
               ++first;
            }

            std::uninitialized_copy( first, last, old_end );
         }

         return updated_pos;
      }

      iterator insert(
         const_iterator pos, std::initializer_list<value_type> init_list ) requires std::copyable<value_type>
      {
         return insert( pos, init_list.begin( ), init_list.end( ) );
      }

      template <class... args_>
      iterator emplace( const_iterator pos, args_&&... args ) requires std::constructible_from<value_type, args_...>
      {
         if ( pos == cend( ) )
         {
            emplace_back( std::forward<args_>( args )... );

            return end( ) - 1;
         }

         assert( pos >= cbegin( ) && "Insertion iterator is out of bounds" );
         assert( pos <= cend( ) && "Insertion iterator is past the end" );

         iterator new_pos;
         if ( super::size( ) >= super::capacity( ) )
         {
            size_type offset = pos - cbegin( );
            grow( );
            new_pos = begin( ) + offset;
         }
         else
         {
            new_pos = begin( ) + ( pos - cbegin( ) );
         }

         new ( static_cast<void*>( &( *end( ) ) ) ) value_type( std::move( back( ) ) );

         std::move_backward( new_pos, end( ) - 1, end( ) );

         ++super::count;

         *new_pos = value_type( std::forward<args_>( args )... );

         return new_pos;
      }

      /**
       * @brief Erases the specified elements from the container.
       *
       * @details Erases the element at pos. Invalidates iterators and references at or after the point of the erase,
       * including the end() iterator. The iterator pos must be valid and dereferenceable. Thus the end() iterator
       * (which is valid, but is not dereferencable) cannot be used as a value for pos.
       *
       * @param[in] pos The iterator to the element to remove.
       *
       * @return An iterator following the last removed element.
       */
      iterator erase( const_iterator pos )
      {
         if ( pos == cend( ) )
         {
            return end( );
         }

         assert( pos >= cbegin( ) && "Insertion iterator is out of bounds" );
         assert( pos <= cend( ) && "Insertion iterator is past the end" );

         auto it = begin( ) + ( pos - cbegin( ) );

         std::move( it + 1, end( ), it );
         pop_back( );

         return it;
      }

      /**
       * @brief Erases the specified elements from the container.
       *
       * @details Removes the elements in the range [first, last). Invalidates iterators and references at or after the
       * point of the erase, including the end() iterator. The iterator first does not need to be dereferenceable if
       * first==last: erasing an empty range is a no-op.
       *
       * @param[in] first The first element inclusive of the range.
       * @param[in] last The last element exclusive of the range.
       *
       * @return Iterator following the last removed element.
       */
      iterator erase( const_iterator first, const_iterator last )
      {
         if ( first == last )
         {
            return begin( ) + ( first - cbegin( ) );
         }

         assert( first >= cbegin( ) && "first iterator is out of bounds" );
         assert( last <= cend( ) && "last iterator is past the end" );
         assert( first <= last && "first iterator is greater than last iterator" );

         size_type const distance = std::distance( first, last );

         auto it_f = begin( ) + ( first - cbegin( ) );
         auto it_l = begin( ) + ( last - cbegin( ) );
         auto it = std::move( it_l, end( ), it_f );

         destroy_range( it, end( ) );

         super::count -= distance;

         return it_f;
      }

      /**
       * @brief Appends the given element value to the end of the container.
       *
       * @details Initialize a new element as a copy of value at the end of the container. If the new size() is greater
       * than capacity() then all iterators and references (including the past-the-end iterator) are invalidated.
       * Otherwise only the past-the-end iterator is invalidated. #value_type must meet the
       * <a href="https://en.cppreference.com/w/cpp/concepts/movable">std::movable</a> requirements to use this function
       *
       * @param[in] value The value of the element to append.
       */
      void push_back( const_reference value ) requires std::copyable<value_type>
      {
         if ( super::size( ) >= super::capacity( ) )
         {
            grow( );
         }

         if constexpr ( trivial_type<value_type> )
         {
            memcpy( static_cast<void*>( &( *end( ) ) ), &value, sizeof( value_type ) );
         }
         else
         {
            new ( static_cast<void*>( &( *end( ) ) ) ) value_type( value );
         }

         ++super::count;
      };

      /**
       * @brief Appends the given element value to the end of the container.
       *
       * @details Move the value into the new element at the end of the container. If the new size() is greater
       * than capacity() then all iterators and references (including the past-the-end iterator) are invalidated.
       * Otherwise only the past-the-end iterator is invalidated. #value_type must meet the
       * <a href="https://en.cppreference.com/w/cpp/concepts/movable">std::movable</a> requirements to use this function
       *
       * @param[in] value The value of the element to append.
       */
      void push_back( value_type&& value ) requires std::movable<value_type>
      {
         if ( super::size( ) >= super::capacity( ) )
         {
            grow( );
         }

         new ( static_cast<void*>( &( *end( ) ) ) ) value_type( std::move( value ) );

         ++super::count;
      };

      /**
       * @brief Appends a new element at the end of the container.
       *
       * @details Constructs in place a new element at the end of the container. The arguments args... are forwarded to
       * the constructor of the #value_type. If the new size() is greater
       * than capacity() then all iterators and references (including the past-the-end iterator) are invalidated.
       * Otherwise only the past-the-end iterator is invalidated. #value_type must meet the
       * <a href="https://en.cppreference.com/w/cpp/concepts/constructible_from">std::constructible_from<value_type,
       * args_...></a> requirements to use this function
       *
       * @tparam args_ The types of the arguments to construct the #value_type from.
       *
       * @param args The arguments to forward to the constructor of the #value_type.
       *
       * @return A #reference to the newly constructed element.
       */
      template <class... args_>
      reference emplace_back( args_&&... args ) requires std::constructible_from<value_type, args_...>
      {
         if ( super::size( ) >= super::capacity( ) )
         {
            grow( );
         }

         iterator last = end( );
         new ( static_cast<void*>( &( *last ) ) ) value_type( std::forward<args_>( args )... );

         ++super::count;

         return *last;
      }

      /**
       * @brief Removes the last element of the container.
       *
       * @brief Removels the last element in the container unless empty. Iterators and references to the last element,
       * as well as the end() iterator, are invalidated.
       */
      void pop_back( )
      {
         if ( super::count != 0 )
         {
            --super::count;
            end( )->~value_type( );
         }
      };

      /**
       * @brief Resizes the container to container count elements.
       *
       * @details  Resizes the container to container count elements. If the current size is greater that count, the
       * container is reduced to its first count elements. If the current size is less than count, additional
       * default-inserted elements are appended. #value_type must meet the
       * <a href="https://en.cppreference.com/w/cpp/concepts/default_initializable">std::default_initializable</a>
       * requirements to use this function
       *
       * @param[in] count The new size of the container.
       */
      void resize( size_type count ) requires std::default_initializable<value_type>
      {
         if ( super::size( ) > count )
         {
            destroy_range( begin( ) + count, end( ) );
            super::count = count;
         }
         else if ( super::size( ) < count )
         {
            if ( super::capacity( ) < count )
            {
               grow( count );
            }

            for ( int i = super::size( ); i < count; ++i )
            {
               new ( static_cast<void*>( static_cast<pointer>( super::p_begin ) + i ) ) value_type( );
            }

            super::count = count;
         }
      }

      /**
       * @brief Resizes the container to container count elements.
       *
       * @details  Resizes the container to container count elements. If the current size is greater that count, the
       * container is reduced to its first count elements. If the current size is less than count, additional
       * copies of value are appended. #value_type must meet the
       * <a href="https://en.cppreference.com/w/cpp/concepts/default_initializable">std::default_initializable</a>
       * requirements to use this function
       *
       * @param[in] count The new size of the container.
       * @param[in] value The value to initialize the new elements with.
       */
      void resize( size_type count, const_reference value ) requires std::copyable<value_type>
      {
         if ( super::size( ) > count )
         {
            destroy_range( begin( ) + count, end( ) );
            super::count = count;
         }
         else if ( super::size( ) < count )
         {
            if ( super::capacity( ) < count )
            {
               grow( count );
            }

            std::uninitialized_fill( end( ), begin( ) + count, value );

            super::count = count;
         }
      }

   private:
      void* get_first_element( ) const noexcept
      {
         using layout = tiny_dynamic_array_align_and_size<value_type, allocator_type>;

         return const_cast<void*>( reinterpret_cast<void const*>(
            reinterpret_cast<char const*>( this ) + offsetof( layout, first_element ) ) );
      }

      void grow( size_type min_size = 0 )
      {
         assert( super::p_alloc != nullptr );

         if ( min_size > std::numeric_limits<difference_type>::max( ) )
         {
            handle_bad_alloc_error( "Hybrid dynamic_array capacity overflow during allocation." );
         }

         if constexpr ( trivial_type<value_type> )
         {
            super::grow_trivial( get_first_element( ), min_size, sizeof( value_type ), alignof( value_type ) );
         }
         else
         {
            size_type new_cap =
               std::clamp( 2 * super::capacity( ) + 1, min_size, std::numeric_limits<size_type>::max( ) );

            pointer p_new{ nullptr };
            if ( is_static( ) ) // memory is currently static
            {
               p_new = static_cast<pointer>(
                  super::p_alloc->allocate( new_cap * sizeof( value_type ), alignof( value_type ) ) );

               if ( !p_new )
               {
                  handle_bad_alloc_error( "hybrid dynamic_array allocation error" );
               }

               if constexpr ( std::movable<value_type> )
               {
                  std::uninitialized_move( begin( ), end( ), iterator{ p_new } );
               }
               else
               {
                  std::uninitialized_copy( begin( ), end( ), iterator{ p_new } );
               }

               destroy_range( begin( ), end( ) );
            }
            else
            {
               // The templated reallocate move the data if needed, so no need to move here.
               p_new =
                  super::p_alloc->template reallocate<value_type>( static_cast<pointer>( super::p_begin ), new_cap );

               if ( !p_new )
               {
                  handle_bad_alloc_error( "Hybrid dynamic_array allocation error" );
               }
            }

            super::p_begin = static_cast<void*>( p_new );
            super::cap = new_cap;
         }
      }

      static void destroy_range( iterator first, iterator last )
      {
         if constexpr ( !trivial_type<value_type> )
         {
            while ( first != last )
            {
               first->~value_type( );
               ++first;
            }
         }
      }
   };

   /**
    * @class tiny_dynamic_array dynamic_array.hpp <ESL/containers/dynamic_array.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, April 29th, 2020
    * @copyright MIT License.
    * @brief A dynamic array data structure that allows for a small static memory storage.
    *
    * @tparam any_, The type of objects that can be contained in the container.
    * @tparam buff_sz, The size of the static memory buffer in the container.
    * @tparam allocator_ The type of the allocator used by the container.
    */
   template <class any_, std::size_t buff_sz, complex_allocator<any_> allocator_ = ESL::multipool_allocator>
   class tiny_dynamic_array : public tiny_dynamic_array_impl<any_, allocator_>, details::static_array_storage<any_, buff_sz>
   {
      using super = tiny_dynamic_array_impl<any_, allocator_>;
      using storage = details::static_array_storage<any_, buff_sz>;

   public:
      using value_type = typename super::value_type;
      using allocator_type = typename super::allocator_type;
      using reference = value_type&;
      using const_reference = value_type const&;
      using pointer = value_type*;
      using const_pointer = value_type const*;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using iterator = random_access_iterator<value_type>;
      using const_iterator = random_access_iterator<value_type const>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      explicit tiny_dynamic_array( allocator_type* p_alloc ) : super( buff_sz, p_alloc ) {}
      explicit tiny_dynamic_array( size_type count, allocator_type* p_alloc ) requires std::default_initializable<value_type> :
         super( buff_sz, p_alloc )
      {
         super::assign( count, value_type( ) );
      }
      tiny_dynamic_array( size_type count, const_reference value,
         allocator_type* p_alloc ) requires std::copyable<value_type> : super( buff_sz, p_alloc )
      {
         super::assign( count, value );
      }
      template <std::input_iterator it_>
      tiny_dynamic_array( it_ first, it_ last, allocator_type* p_alloc ) requires std::copyable<value_type> :
         super( buff_sz, p_alloc )
      {
         super::assign( first, last );
      }
      tiny_dynamic_array( std::initializer_list<any_> init, allocator_type* p_alloc ) requires std::copyable<value_type> :
         super( buff_sz, p_alloc )
      {
         super::assign( init );
      }
      tiny_dynamic_array( tiny_dynamic_array const& other ) : super( buff_sz, other.p_alloc )
      {
         if ( !other.empty( ) )
         {
            super::operator=( other );
         }
      }
      tiny_dynamic_array( tiny_dynamic_array const& other, allocator_type* p_alloc ) : super( buff_sz, p_alloc )
      {
         if ( !other.empty( ) )
         {
            super::operator=( other );
         }
      }
      tiny_dynamic_array( tiny_dynamic_array&& other ) : super( buff_sz, other.p_alloc )
      {
         if ( !other.empty( ) )
         {
            super::operator=( std::move( other ) );
         }
      }

      tiny_dynamic_array& operator=( tiny_dynamic_array const& other )
      {
         super::operator=( other );
         return *this;
      }

      tiny_dynamic_array& operator=( super const& other )
      {
         super::operator=( other );
         return *this;
      }

      tiny_dynamic_array& operator=( tiny_dynamic_array&& other )
      {
         super::operator=( std::move( other ) );
         return *this;
      }

      tiny_dynamic_array& operator=( super&& other )
      {
         super::operator=( std::move( other ) );
         return *this;
      }
   };

   template <class any_, complex_allocator<any_> allocator_ = ESL::multipool_allocator>
   using dynamic_array = tiny_dynamic_array<any_, 0, allocator_>;
} // namespace ESL
