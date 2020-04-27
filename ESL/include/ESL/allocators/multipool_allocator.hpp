/**
 * MIT License
 *
 * Copyright (c) 2020 Wmbat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <ESL/allocators/allocator_utils.hpp>

#include <cassert>
#include <memory>

namespace ESL
{
   /**
    * @class multipool_allocator multipool_allocator.hpp <ESL/allocators/multipool_allocator.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Tuesday April 7th, 2020
    * @brief A pool allocator holding multiple layers of pools with varying sizes.
    * @copyright MIT license
    */
   class multipool_allocator final
   {
      struct block_header
      {
         block_header* p_next = nullptr;
         std::size_t depth_index = 0;
      };

      struct access_header
      {
         block_header* p_first_free = nullptr;
      };

   public:
      using pointer = void*;
      using size_type = std::size_t;

      /**
       * @brief The data used to create the #multipool_allocator.
       */
      struct create_info
      {
         size_type pool_count{ 1 };
         size_type pool_size{ 1024 };
         size_type depth{ 1 };
      };

   public:
      /**
       * @brief Constructs an empty allocator with no memory.
       */
      multipool_allocator( ) = default;
      /**
       * @brief Constructs an allocator with the given parameters in the #create_info struct.
       *
       * @param[in]  create_info    The parameters used to construct the allocator and allocate the memory.
       */
      multipool_allocator( create_info const& create_info ) noexcept;

      /**
       * @brief Give out a pool of memory that best matchdes the size requirement.
       *
       * @param[in]  size        The desired size of the memory pool.
       * @param[in]  alignment   The memory alignment of the pool.
       *
       * @return A pointer to the memory pool.
       */
      [[nodiscard( "Memory will go to waste" )]] pointer allocate(
         size_type size, size_type alignment = alignof( std::max_align_t ) ) noexcept;
   
      /**
       * @brief Doesn't really do much.
       *
       * @param[in]  p_alloc        The memory allocation to reallocate.
       * @param[in]  size           The desired size of the pool to give out.
       * @return Returns nullptr if the new_size is greater than the pool size, otherwise p_alloc.
       */
      [[nodiscard( "Memory will go to waste" )]] pointer reallocate( pointer p_alloc, size_type new_size ) noexcept;

      /**
       * @brief Give the memory pool back to the allocator.
       *
       * @param[in]  p_alloc     The pointer to the previously given memory.
       */
      void deallocate( pointer p_alloc ) noexcept;

      /**
       * @brief Return the size of a pool.
       *
       * @param[in]  p_alloc     The allocation of memory associated with a pool.
       *
       * @return The size of a pool the memory lives in.
       */
      size_type allocation_capacity( pointer alloc ) const noexcept;

      /**
       * @brief Reset the allocator's memory. All pointers received from the allocator become invalid.
       */
      void release( ) noexcept;

      /**
       * @brief Return the size of the allocator's memory.
       *
       * @return The size of the allocator's memory.
       */
      size_type max_size( ) const noexcept;
      /**
       * @brief Return the amount of memory that has been given out.
       */
      size_type memory_usage( ) const noexcept;
      /**
       * @brief Return the amount of times memory has been given out.
       */
      size_type allocation_count( ) const noexcept;

       

      /**
       * @brief Constructs an instance of type type_ in a pool of memory.
       *
       * @tparam     type_       The type of the object ot construct in the pool.
       * @tparam     args_       The parameters used to construct an instance of type type_.
       *
       * @param[in]  args        The parameters necessary for the construction of an object of type type_.
       *
       * @return A pointer to the newly constructed object in a memory pool.
       */
      template <class type_, class... args_>
      [[nodiscard( "Memory will go to waste" )]] type_* construct( args_&&... args ) noexcept
         requires std::constructible_from<type_, args_...>
      {
         if ( auto* p_alloc = allocate( sizeof( type_ ), alignof( type_ ) ) )
         {
            return new ( p_alloc ) type_( args... );
         }
         else
         {
            return nullptr;
         }
      }

      /**
       * @brief Destroy an object previously allocated by the allocator and free it's memory pool.
       *
       * @param[in]  p_type   A pointer to the object's memory.
       */
      template <class type_>
      void destroy( type_* p_type ) noexcept
      {
         if ( p_type )
         {
            p_type->~type_( );
            free( TO_BYTE_PTR( p_type ) );
         }
      }

      template <std::default_initializable type_>
      [[nodiscard( "Memory will go to waste" )]] type_* construct_array( size_type count ) noexcept
      {
         assert( count != 0 );

         auto* p_data = reinterpret_cast<type_*>( allocate( sizeof( type_ ) * count, alignof( type_ ) ) );
         if ( !p_data )
         {
            return nullptr;
         }

         for ( size_type i = 0; i < count; ++i )
         {
            new ( p_data + i ) type_( );
         }

         return p_data;
      }

      template <class type_>
      [[nodiscard( "Memory will go to waste" )]] type_* construct_array( size_type count, type_ const& value ) noexcept
      {
         assert( count != 0 );

         auto* p_data = reinterpret_cast<type_*>( allocate( sizeof( type_ ) * count, alignof( type_ ) ) );
         if ( !p_data )
         {
            return nullptr;
         }

         for ( size_type i = 0; i < count; ++i )
         {
            new ( p_data + i ) type_( value );
         }

         return p_data;
      }

      template <class type_>
      void destroy_array( type_* const p_data, size_type count ) noexcept
      {
         assert( count != 0 );

         for ( size_type i = 0; i < count; ++i )
         {
            p_data[i].~type_( );
         }

         deallocate( TO_BYTE_PTR( p_data ) );
      }

      /**
       * @brief Create a unique handle to a object allocated in the allocator, it'll be automatically destroyed up
       * reaching the end of the handle's scope.
       *
       * @tparam     type_       The type of the object ot construct in the pool.
       * @tparam     args_       The parameters used to construct an instance of type type_.
       *
       * @param[in]  args        The parameters necessary for the construction of an object of type type_.
       *
       * @return A unique pointer to the newly constructed object in a memory pool.
       */
      template <class type_, class... args_>
      [[nodiscard]] auto_ptr<type_> make_unique( args_&&... args ) noexcept
         requires std::constructible_from<type_, args_...>
      {
         return auto_ptr<type_>( construct<type_>( args... ), [this]( type_* p_type ) {
            this->destroy( p_type );
         } );
      }

   private:
      size_type pool_count{ 0 };
      size_type pool_size{ 0 };
      size_type depth{ 1 };

      size_type total_size{ 0 };
      size_type used_memory{ 0 };
      size_type num_allocations{ 0 };

      std::unique_ptr<std::byte[]> p_memory{ nullptr };
      access_header* p_access_headers{ nullptr };
   };
} // namespace ESL
