/**
 * @file avl_map.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, April 23rd, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <ESL/allocators/allocator_utils.hpp>
#include <ESL/allocators/multipool_allocator.hpp>
#include <ESL/utils/compare.hpp>
#include <ESL/utils/concepts.hpp>
#include <ESL/utils/iterators/random_access_iterator.hpp>

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
    * @class vector_base vector.hpp <ESL/containers/vector.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, April 22th, 2020
    * @copyright MIT License.
    * @brief A common base for all dynamic arrays using custom allocators.
    *
    * @tparam allocator_ The type of the allocator used by the container.
    */
   template <full_allocator allocator_>
   class vector_base
   {
   public:
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = allocator_;
      using pointer = void*;

   protected:
      /**
       * @brief deleted default constructor.
       */
      vector_base( ) = delete;
      vector_base( pointer p_first_el, size_type capacity, allocator_type* p_alloc ) :
         p_begin( p_first_el ), cap( capacity ), p_alloc( p_alloc )
      {}

      void grow_pod( void* p_first_element, size_type min_cap, size_type type_size, size_type type_align )
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
      size_type capacity( ) const noexcept { return cap; }

   protected:
      void* p_begin{ nullptr };

      size_type count{ 0 };
      size_type cap{ 0 };

      allocator_type* p_alloc{ nullptr };
   };

   template <class any_, complex_allocator<any_> allocator_>
   struct hybrid_vector_align_and_size
   {
      std::aligned_storage_t<sizeof( vector_base<allocator_> ), alignof( vector_base<allocator_> )> base;
      std::aligned_storage_t<sizeof( any_ ), alignof( any_ )> first_element;
   };

   template <class any_, std::size_t buff_sz>
   struct static_vector_storage
   {
      std::aligned_storage_t<sizeof( any_ ), alignof( any_ )> data[buff_sz];
   };

   template <class any_>
   struct alignas( alignof( any_ ) ) static_vector_storage<any_, 0>
   {
   };

   template <class any_, complex_allocator<any_> allocator_>
   class hybrid_vector_impl : public vector_base<allocator_>
   {
      using super = vector_base<allocator_>;

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
      hybrid_vector_impl( ) = delete;
      explicit hybrid_vector_impl( size_type size, allocator_type* p_alloc ) :
         super( get_first_element( ), size, p_alloc )
      {}

      bool is_static( ) const noexcept { return super::p_begin == get_first_element( ); }

   public:
      virtual ~hybrid_vector_impl( )
      {
         if ( !is_static( ) )
         {
            clear( );

            super::p_alloc->deallocate( super::p_begin );
         }
      }

      /**
       * @brief Return a reference to the element at the index position in the container.
       *
       * @throw std::out_of_range
       *
       * @param[in]  index    The index position of the desired element.
       *
       * @return A #reference to the element at the index position in the container.
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
       * @brief Return a reference to the element at the index position in the container.
       *
       * @throw std::out_of_range
       *
       * @param[in]  index    The index position of the desired element.
       *
       * @return A #const_reference to the element at the index position in the container.
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

      reference operator[]( size_type index ) noexcept
      {
         assert( index < super::count && "Index out of bounds." );
         return *super::p_begin[index];
      }
      const_reference operator[]( size_type index ) const noexcept
      {
         assert( index < super::count && "Index out of bounds." );
         return *super::p_begin[index];
      }

      reference front( ) noexcept
      {
         assert( !super::empty( ) && "No elements in the container" );
         return *begin( );
      }
      const_reference front( ) const noexcept
      {
         assert( !super::empty( ) && "No elements in the container" );
         return *cbegin( );
      }

      reference back( ) noexcept
      {
         assert( !super::empty( ) && "No elements in the container" );
         return *( end( ) - 1 );
      }
      const_reference back( ) const noexcept
      {
         assert( !super::empty( ) && "No elements in the container" );
         return *( cend( ) - 1 );
      }

      pointer data( ) noexcept { return pointer{ begin( ) }; }
      const_pointer data( ) const noexcept { return const_pointer{ cbegin( ) }; }

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

      void reserve( size_type new_cap )
      {
         if ( new_cap > super::capacity )
         {
            grow( new_cap );
         }
      }

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

            return cend( ) - 1;
         }

         assert( pos >= cbegin( ) && "Insertion iterator is out of bounds" );
         assert( pos <= cend( ) && "Insertion iterator is past the end" );

         if ( super::size( ) >= super::capacity( ) )
         {
            grow( );
         }
      }
      /*
      iterator insert( const_iterator pos, value_type&& value ) requires std::movable<value_type> {}
      iterator insert( const_iterator pos, size_type count, reference value ) requires std::copyable<value_type> {}
      iterator insert( const_iterator pos, std::input_iterator auto first, std::input_iterator auto last ) {}
      iterator insert( const_iterator pos, std::initializer_list<value_type> initializer_list ) {}

      template <class... args_>
      iterator emplace( const_iterator pos, args_&&... args ) requires std::constructible_from<value_type, args_...>
      {}

      iterator erase( const_iterator pos );
      iterator erase( const_iterator first, const_iterator last );
      */

      void push_back( const_reference value ) requires std::copyable<value_type>
      {
         if ( super::size( ) >= super::capacity( ) )
         {
            grow( );
         }

         if constexpr ( pod_type<value_type> )
         {
            memcpy( static_cast<void*>( *end( ) ), &value, sizeof( value_type ) );
         }
         else
         {
            new ( static_cast<void*>( &( *end( ) ) ) ) value_type( value );
         }

         ++super::count;
      };
      void push_back( value_type&& value ) requires std::movable<value_type>
      {
         if ( super::size( ) >= super::capacity( ) )
         {
            grow( );
         }

         new ( static_cast<void*>( &( *end( ) ) ) ) value_type( std::move( value ) );

         ++super::count;
      };

      /*
      template <class... args_>
      reference emplace_back( args_&&... args ) requires std::constructible_from<value_type, args_...>
      {}

      void pop_back( ){ };

      void resize( size_type count ) requires std::default_initializable<value_type>;
      void resize( size_type count, const_reference value ) requires std::copyable<value_type> {}
*/
   private:
      void* get_first_element( ) const noexcept
      {
         using layout = hybrid_vector_align_and_size<value_type, allocator_type>;

         return const_cast<void*>(
            static_cast<void const*>( reinterpret_cast<char const*>( this ) + offsetof( layout, first_element ) ) );
      }

      void grow( size_type min_size = 0 )
      {
         if ( min_size > std::numeric_limits<difference_type>::max( ) )
         {
            // handle error
         }

         if constexpr ( pod_type<value_type> )
         {
            super::grow_pod( get_first_element( ), min_size, sizeof( value_type ) );
         }
         else
         {
            size_type new_cap =
               std::clamp( 2 * super::capacity( ) + 1, min_size, std::numeric_limits<size_type>::max( ) );

            if ( begin( ) == end( ) )
            {
               pointer p_new = static_cast<pointer>(
                  super::p_alloc->allocate( new_cap * sizeof( value_type ), alignof( value_type ) ) );
            }
            else
            {
            }

            /*
            pointer p_new = static_cast<pointer>(
               super::p_alloc->allocate( new_cap * sizeof( value_type ), alignof( value_type ) ) );

            if constexpr ( std::movable<value_type> )
            {
               std::uninitialized_move( begin( ), end( ), iterator{ p_new } );
            }
            else
            {
               std::uninitialized_copy( begin( ), end( ), iterator{ p_new } );
            }

            destroy_range( begin( ), end( ) );

            if ( !this->is_static( ) )
            {
               super::p_alloc->deallocate( static_cast<void*>( &( *begin( ) ) ) );
            }
            */

            super::cap = new_cap;
         }
      }

      static void destroy_range( iterator first, iterator last )
      {
         if constexpr ( !pod_type<value_type> )
         {
            while ( first != last )
            {
               first->~value_type( );
               ++first;
            }
         }
      }
   };

   template <class any_, std::size_t buff_sz, complex_allocator<any_> allocator_ = ESL::multipool_allocator>
   class hybrid_vector : public hybrid_vector_impl<any_, allocator_>, static_vector_storage<any_, buff_sz>
   {
      using super_impl = hybrid_vector_impl<any_, allocator_>;
      using super_storage = static_vector_storage<any_, buff_sz>;

   public:
      using value_type = typename super_impl::value_type;
      using allocator_type = typename super_impl::allocator_type;
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
      explicit hybrid_vector( allocator_type* p_alloc ) : super_impl( buff_sz, p_alloc ) {}
      /*
      explicit hybrid_vector( size_type count, reference value, allocator_type* p_alloc ) {}
      explicit hybrid_vector( size_type count, allocator_type* p_alloc ) requires std::default_initializable<value_type>
      {}
      hybrid_vector( std::input_iterator auto first, std::input_iterator auto last, allocator_type* p_alloc ) {}
      hybrid_vector( std::initializer_list<any_> init, allocator_type* p_alloc ) {}
      hybrid_vector( hybrid_vector const& other ) {}
      hybrid_vector( hybrid_vector const& other, allocator_type* p_alloc ) {}
      hybrid_vector( hybrid_vector&& other ) {}
      hybrid_vector( hybrid_vector&& other, allocator_type* p_alloc ) {}
      */
   };

   template <class any_, complex_allocator<any_> allocator_ = ESL::multipool_allocator>
   using tvector = hybrid_vector<any_, 0, allocator_>;

#define TO_TYPE_PTR( ptr ) reinterpret_cast<pointer>( ptr )

   /**
    * @class vector vector.hpp <ESL/containers/vector.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, April 4th, 2020
    * @brief A dynamically allocated resizable array.
    * @copyright MIT License.
    *
    * @tparam  type_       The type of the vector's elements.
    * @tparam  allocator_  The type of the allocator.
    */
   template <class type_, complex_allocator<type_> allocator_ = ESL::multipool_allocator>
   class vector
   {
      using is_ptr = std::is_pointer<type_>;

   public:
      using value_type = type_;
      using allocator_type = allocator_;
      using reference = value_type&;
      using const_reference = value_type const&;
      using pointer = value_type*;
      using const_pointer = value_type const*;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using iterator = random_access_iterator<type_>;
      using const_iterator = random_access_iterator<type_ const>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      /**
       * @brief Constructs an empty vector with a pointer to an #allocator.
       *
       * @param[in]  p_allocator*   A pointer to the allocator that will hold the vector's memory.
       */
      explicit vector( allocator_* p_allocator ) noexcept : p_allocator( p_allocator )
      {
         assert( p_allocator != nullptr && "Cannot have a nullptr allocator" );
      }
      /**
       * @brief Constructs a vector with count copies of elements with value value and a pointer to
       * an allocator.
       *
       * Constructs a vector of size count with value value of type #value_type in a memory location provided
       * by the allocator of type #allocator_type. To use this constructor, the type #value_type must comply
       * with the <a href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a> concept.
       *
       * @throw   std::bad_alloc    Thrown if the allocator fails to allocate memory. If the exception occurs, the
       * state of the vector instance will remain unchanged and valid.
       *
       * @param[in]  count          The number of elements to place in the vector.
       * @param[in]  value          The value to assign the elements placed in the vector.
       * @param[in]  p_allocator    A pointer to the allocator that will hold the vector's memory.
       */
      explicit vector(
         size_type count, const_reference value, allocator_type* p_allocator ) requires std::copyable<value_type> :
         p_allocator( p_allocator ),
         current_capacity( count ),
         current_size( count )
      {
         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         p_alloc = p_allocator->template construct_array<value_type>( current_size, value );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }
      }

      /**
       * @brief Constructs a vector with count copies of elements with default value and a pointer to an
       * allocator.
       *
       * Constructs a vector of size count with elements of type value_type in a memory location provided
       * by the allocator of type #allocator_type.
       * To use this constructor, the type value_type must comply
       * with the <a
       * href="https://en.cppreference.com/w/cpp/concepts/default_initializable">std::default_initializable</a> concept.
       *
       * @throw   std::bad_alloc    Thrown if the allocator fails to allocate memory. If the exception occurs, the
       * state of the vector instance will remain unchanged and valid.
       *
       * @param[in]  count          The number of elements to place in the vector.
       * @param[in]  p_allocator    A pointer to the allocator that will hold the vector's memory.
       */
      explicit vector( size_type count, allocator_type* p_allocator ) requires std::default_initializable<value_type> :
         p_allocator( p_allocator ),
         current_capacity( count ),
         current_size( count )
      {
         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         p_alloc = p_allocator->template construct_array<value_type>( current_size );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }
      }
      /**
       * @brief Constructs a vector from a range of input iterators [first, last) and a pointer to an allocator.
       *
       * Constructs a vector using a range designated by input iterators from first to last exclusive on a memory
       * location provided by the allocator of type #allocator_type. To use this constructor, the type #value_type
       * must comply with the <a href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a>
       * concept.
       *
       * @throw   std::bad_alloc    Thrown if the allocator fails to allocate memory, If the exception occurs, the
       * state of the vector instance will remain unchanged and valid.
       *
       * @param[in]  first          The first element to start copying from. Must satisfy the std::input_iterator
       * requirement.
       * @param[in]  last           The last element to end copying from. Must satisfy the std::input_iterator
       * requirement.
       * @param[in]  p_allocator    A pointer to the allocator that will hold the vector's memory.
       */
      explicit vector( std::input_iterator auto first, std::input_iterator auto last,
         allocator_type* p_allocator ) requires std::copyable<value_type> :
         p_allocator( p_allocator ),
         current_capacity( last - first ),
         current_size( current_capacity )
      {
         assert( p_allocator != nullptr && "Allocator cannot be nullptr" );

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         for ( size_type i = 0; first != last; ++i, ++first )
         {
            p_alloc[i] = *first;
         }
      }
      /**
       * @brief Constructs a vector from an <a
       * href="https://en.cppreference.com/w/cpp/utility/initializer_list">std::initializer_list</a> and a pointer to an
       * allocator.
       *
       * @details Constructs a vector from an
       * <a href="https://en.cppreference.com/w/cpp/utility/initializer_list">std::initializer_list</a> and place all
       * elements of the list into a memory location provided by the allocator. To use this constructor, the type
       * value_type must satisfy the
       * <a href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a> requirement.
       *
       * @throw   std::bad_alloc    Thrown if the allocator fails to allocate memory, If the exception occurs, the
       * state of the vector instance will remain unchanged and valid.
       *
       * @param[in]  init           The list of elements to place into the vector.
       * @param[in]  p_allocator    A pointer to the allocator that will hold the vector's memory.
       */
      vector( std::initializer_list<type_> init, allocator_type* p_allocator ) requires std::copyable<value_type> :
         current_capacity( init.size( ) ),
         current_size( init.size( ) ),
         p_allocator( p_allocator )
      {
         assert( p_allocator != nullptr && "allocator cannot be nullptr" );

         p_alloc = p_allocator->template construct_array<type_>( current_size );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         for ( size_type index = 0; auto& it : init )
         {
            p_alloc[index++] = it;
         }
      }
      /**
       * @brief Construct a vector from another vector.
       *
       * @details Construct a vector from another vector, copying it's data into a new memory allocation from the same
       * allocator. To use this constructor, the type #value_type must satisfy the <a
       * href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a> requirement.
       *
       * @throw   std::bad_alloc    Thrown if the allocator fails to allocate memory, If the exception occurs, the
       * state of the vector instance will remain unchanged and valid.
       *
       * @param[in]  other    The vector to copy the data from.
       */
      vector( vector const& other ) requires std::copyable<value_type>
      {
         p_allocator = other.p_allocator;
         current_size = other.current_size;
         current_capacity = other.current_capacity;

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         for ( size_type i = 0; i < current_size; ++i )
         {
            p_alloc[i] = other[i];
         }
      }
      /**
       * @brief Construct a vector from another vector.
       *
       * @details Construct a vector from another vector, copying it's data into a new memory allocation from another
       * allocator. To use this constructor, the type #value_type must satisfy the <a
       * href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a> requirement.
       *
       * @throw   std::bad_alloc    Thrown if the allocator fails to allocate memory, If the exception occurs, the
       * state of the vector instance will remain unchanged and valid.
       *
       * @param[in]  other          The vector to copy the data from.
       * @param[in]  p_allocator    A pointer to the allocator that will hold the vector's memory.
       */
      vector( vector const& other, allocator_type* p_allocator ) requires std::copyable<value_type> :
         p_allocator( p_allocator )
      {
         if ( !p_allocator )
         {
            p_allocator = other.p_allocator;
         }

         current_size = other.size( );
         current_capacity = other.size( );

         auto const size_in_bytes = current_capacity * sizeof( value_type );
         p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
         if ( !p_alloc )
         {
            throw std::bad_alloc( );
         }

         std::copy( other.cbegin( ), other.cend( ), p_alloc );
      }
      /**
       * @brief Move the data from another vector into this vector.
       *
       * @param[in]  other    The vector to move the data from.
       */
      vector( vector&& other ) noexcept
      {
         p_allocator = other.p_allocator;
         other.p_allocator = nullptr;

         current_size = other.current_size;
         other.current_size = 0;

         current_capacity = other.current_capacity;
         other.current_capacity = 0;

         p_alloc = other.p_alloc;
         other.p_alloc = nullptr;
      }
      /**
       * @brief Destructor.
       */
      ~vector( )
      {
         if ( p_allocator && p_alloc )
         {
            if constexpr ( !pod_type<value_type> )
            {
               for ( size_type i = 0; i < current_size; ++i )
               {
                  p_alloc[i].~type_( );
               }
            }

            p_allocator->deallocate( TO_BYTE_PTR( p_alloc ) );
            p_alloc = nullptr;
         }

         p_allocator = nullptr;
      }

      vector& operator=( vector const& other ) requires std::copyable<value_type>
      {
         if ( this != &other )
         {
            p_allocator = other.p_allocator;
            current_size = other.current_size;
            current_capacity = other.current_capacity;

            auto const size_in_bytes = current_capacity * sizeof( value_type );
            p_alloc = TO_TYPE_PTR( p_allocator->allocate( size_in_bytes, alignof( value_type ) ) );
            if ( !p_alloc )
            {
               throw std::bad_alloc( );
            }

            for ( size_type i = 0; i < current_size; ++i )
            {
               p_alloc[i] = other[i];
            }
         }

         return *this;
      }
      vector& operator=( vector&& other ) noexcept
      {
         if ( this != &other )
         {
            p_allocator = other.p_allocator;
            other.p_allocator = nullptr;

            current_size = other.current_size;
            other.current_size = 0;

            current_capacity = other.current_capacity;
            other.current_capacity = 0;

            p_alloc = other.p_alloc;
            other.p_alloc = nullptr;
         }

         return *this;
      }
      vector& operator=( std::initializer_list<value_type> init ) requires std::copyable<value_type>
      {
         if ( !p_alloc )
         {
            p_alloc =
               TO_TYPE_PTR( p_allocator->allocate( sizeof( value_type ) * init.size( ), alignof( value_type ) ) );
            if ( !p_alloc )
            {
               throw std::bad_alloc{ };
            }
         }
         else
         {
            pointer p_temp{ nullptr };
            if ( current_capacity < init.size( ) )
            {
               current_capacity = p_allocator->allocation_capacity( TO_BYTE_PTR( p_alloc ) ) / sizeof( value_type );

               if ( current_capacity < init.size( ) )
               {
                  p_temp =
                     TO_TYPE_PTR( p_allocator->allocate( sizeof( value_type ) * init.size( ), alignof( value_type ) ) );
                  if ( !p_temp )
                  {
                     throw std::bad_alloc{ };
                  }
               };
            }

            if constexpr ( pod_type<value_type> )
            {
               std::for_each( begin( ), end( ), []( value_type& type ) {
                  type.~value_type( );
               } );
            }

            if ( p_temp )
            {
               p_alloc = p_temp;
               p_temp = nullptr;
            }
         }

         current_size = init.size( );
         current_capacity = init.size( );

         for ( size_type index = 0; auto& it : init )
         {
            p_alloc[index++] = it;
         }

         return *this;
      }

      /**
       * @brief Check if two vectors have the same elements.
       *
       * @tparam  other_   The allocator of the other vector.
       * @param   rhs      The vector to compare against.
       *
       * @return True if the two vectors have the same data, otherwise false
       */
      template <complex_allocator<type_> other_ = allocator_>
      constexpr bool operator==( vector<type_, other_> const& rhs ) const requires std::equality_comparable<type_>
      {
         return std::equal( cbegin( ), cbegin( ), rhs.cbegin( ), rhs.cend( ) );
      }

      /**
       * @brief Perform a lexicographical compare on the elements of the two vectors.
       *
       * @tparam  other_   The allocator of the other vector.
       * @param   rhs      The vector to compare against.
       *
       * @return An ordering defining the relationship between the two vectors.
       */
      template <complex_allocator<type_> other_ = allocator_>
      constexpr auto operator<=>( vector<type_, other_> const& rhs )
      {
         return std::lexicographical_compare_three_way(
            cbegin( ), cend( ), rhs.cbegin( ), rhs.cend( ), synth_three_way );
      }

      /**
       * @brief Assign a value to a count of elements in the container.
       *
       * @details Assign a number count of elements with the value value in the container. if the container requires
       * reallocation, all elements currently present in the container will be moved to the new allocation. This
       * function may only be called if the type #value_type satisfies the
       * <a href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a> requirement.
       *
       * @param[in]  count    The number of elements to assign a value.
       * @param[in]  value    The value to assign to the elements.
       */
      void assign( size_type count, value_type const& value ) requires std::copyable<value_type>
      {
         reallocate( count );

         current_size = count;
         for ( size_type i = 0; i < current_size; ++i )
         {
            p_alloc[i] = value;
         }
      }
      /**
       *
       */
      void assign( std::input_iterator auto first, std::input_iterator auto last ) requires std::copyable<value_type>
      {
         size_type const count = std::distance( first, last );

         reallocate( count );

         current_size = count;
         for ( size_type i = 0; first != last; ++i, ++first )
         {
            p_alloc[i] = *first;
         }
      }
      void assign( std::initializer_list<value_type> init ) requires std::copyable<value_type>
      {
         reallocate( init.size( ) );

         current_size = init.size( );
         for ( size_type i = 0; auto& it : init )
         {
            p_alloc[i++] = it;
         }
      }

      /**
       * @brief Return a pointer to the container's allocator.
       *
       * @return The pointer to the container's allocator
       */
      allocator_type* get_allocator( ) noexcept { return p_allocator; }
      /**
       * @brief Return a pointer to the container's allocator.
       *
       * @return The pointer to the container's allocator
       */
      allocator_type const* get_allocator( ) const noexcept { return p_allocator; }

      /**
       * @brief Return a reference to the element at the index position in the container.
       *
       * @param[in]  index    The index position of the desired element.
       *
       * @return A reference to the element at the index position in the container.
       */
      reference at( size_type index )
      {
         if ( index < 0 && index >= current_size )
         {
            throw std::out_of_range{ "Index: " + std::to_string( index ) + " is out of bounds" };
         }
         else
         {
            return p_alloc[index];
         }
      }
      /**
       * @brief Return a const_reference to the element at the index position in the container.
       *
       * @param[in]  index    The index position of the desired element.
       *
       * @return A const_reference to the element at the index position in the container.
       */
      const_reference at( size_type index ) const
      {
         if ( index < 0 && index >= current_size )
         {
            throw std::out_of_range{ "Index: " + std::to_string( index ) + " is out of bounds" };
         }
         else
         {
            return p_alloc[index];
         }
      }
      /**
       * @brief Return a reference to the element at the desired index position.
       *
       * @param[in]  index    The index position of the element to access.
       *
       * @return A reference to the element at location index.
       */
      reference operator[]( size_type index ) noexcept
      {
         assert( index >= 0 && "Index cannot be less than zero" );
         assert( index < current_size && "Index cannot be more than vector size" );

         return p_alloc[index];
      }
      /**
       * @brief Return a const_reference to the element at the desired index position.
       *
       * @param[in]  index    The index position of the element to access.
       *
       * @return A const_reference to the element at location index.
       */
      const_reference operator[]( size_type index ) const noexcept
      {
         assert( index >= 0 && "Index cannot be less than zero" );
         assert( index < current_size && "Index cannot be more than vector size" );

         return p_alloc[index];
      }
      /**
       * @brief Return a reference to the first element in the container.
       *
       * @return A reference to the first element in the container.
       */
      reference front( ) noexcept { return p_alloc[0]; }
      /**
       * @brief Return a const_reference to the first element in the container.
       *
       * @return A const_reference to the first element in the container.
       */
      const_reference front( ) const noexcept { return p_alloc[0]; }

      /**
       * @brief Return a reference to the last element in the container.
       *
       * @return A reference to the last element in the container.
       */
      reference back( ) noexcept { return p_alloc[current_size - 1]; }
      /**
       * @brief Return a const_reference to the last element in the container.
       *
       * @return A const_reference to the last element in the container.
       */
      const_reference back( ) const noexcept { return p_alloc[current_size - 1]; }

      /**
       * @brief Return a pointer to the memory allocation of the container's data.
       *
       * @return A pointer to the memory allocation of the container's data.
       */
      pointer data( ) noexcept { return p_alloc; }
      /**
       * @brief Return a const_pointer to the memory allocation of the container's data.
       *
       * @return A const_pointer to the memory allocation of the container's data.
       */
      const_pointer data( ) const noexcept { return p_alloc; }

      /**
       * @brief Return an @ref random_access_iterator iterator to the first element in the container.
       *
       * @return The iterator to the first element in the container.
       */
      iterator begin( ) noexcept { return iterator{ p_alloc }; }
      /**
       * @brief Return a const_iterator to the first element in the container.
       *
       * @return The const_iterator to the first element in the container.
       */
      const_iterator begin( ) const noexcept { return const_iterator{ p_alloc }; }
      /**
       * @brief Return a const_iterator to the first element in the container.
       *
       * @return The const_iterator to the first element in the container.
       */
      const_iterator cbegin( ) const noexcept { return const_iterator{ p_alloc }; }

      /**
       * @brief Return a iterator to one past the last element in the container.
       *
       * @return The iterator to one past the last element in the container.
       */
      iterator end( ) noexcept { return iterator{ p_alloc + current_size }; }
      /**
       * @brief Return a const_iterator to one past the last element in the container.
       *
       * @return The const_iterator to one past the last element in the container.
       */
      const_iterator end( ) const noexcept { return const_iterator{ p_alloc + current_size }; }
      /**
       * @brief Return a const_iterator to one past the last element in the container.
       *
       * @return The const_iterator to one past the last element in the container.
       */
      const_iterator cend( ) const noexcept { return const_iterator{ p_alloc + current_size }; }

      /**
       * @brief Return a reverse_iterator to the first element in the container.
       *
       * @return The reverse_iterator to the first element in the container.
       */
      reverse_iterator rbegin( ) noexcept { return reverse_iterator{ p_alloc }; }
      /**
       * @brief Return a const_reverse_iterator to the first element in the container.
       *
       * @return The const_reverse_iterator to the first element in the container.
       */
      const_reverse_iterator rbegin( ) const noexcept { return const_reverse_iterator{ p_alloc }; }
      /**
       * @brief Return a const_reverse_iterator to the first element in the container.
       *
       * @return The const_reverse_iterator to the first element in the container.
       */
      const_reverse_iterator crbegin( ) const noexcept { return const_reverse_iterator{ p_alloc }; }

      /**
       * @brief Return a reverse_iterator to one past the last element in the container.
       *
       * @return The reverse_iterator to one past the last element in the container.
       */
      reverse_iterator rend( ) noexcept { return reverse_iterator{ p_alloc + current_size }; }
      /**
       * @brief Return a const_reverse_iterator to one past the last element in the container.
       *
       * @return The const_reverse_iterator to one past the last element in the container.
       */
      const_reverse_iterator rend( ) const noexcept { return reverse_iterator{ p_alloc + current_size }; }
      /**
       * @brief Return a const_reverse_iterator to one past the last element in the container.
       *
       * @return The const_reverse_iterator to one past the last element in the container.
       */
      const_reverse_iterator crend( ) const noexcept { return const_reverse_iterator{ p_alloc + current_size }; }

      /**
       * @brief Check if the container has no element.
       *
       * @return True if the container is empty, otherwise false.
       */
      constexpr bool empty( ) const noexcept { return current_size == 0; }
      /**
       * @brief Return the number of elements in the container.
       *
       * @return The size of the container.
       */
      constexpr size_type size( ) const noexcept { return current_size; }
      /**
       * @brief Return the maximum size of the container.
       *
       * @return The maximum size of the container.
       */
      constexpr size_type max_size( ) const noexcept { return std::numeric_limits<std::uintptr_t>::max( ); }
      /**
       * @brief Reserve a piece of memory of size new_capacity from the allocator.
       *
       * Reserve a piece of memory of size new_capacity from the allocator. If the container already has elements and
       * the new capacity is greater than the old capacity, all elements will be moved over to the new memory
       * allocation.
       *
       * @throw   std::bad_alloc    Thrown if the allocator fails to allocate memory, If the exception occurs, the
       * state of the vector instance will remain unchanged and valid.
       *
       * @param[in]  new_capacity   The new desired capacity for the container's memory allocation.
       */
      void reserve( size_type new_capacity )
      {
         if ( new_capacity > max_size( ) )
         {
            throw std::length_error{ "number of elements " + std::to_string( new_capacity ) + " is too big" };
         }

         if ( new_capacity > current_capacity )
         {
            auto* p_temp = reinterpret_cast<pointer>( p_allocator->allocate( new_capacity, alignof( value_type ) ) );
            if ( !p_temp )
            {
               throw std::bad_alloc{ };
            }

            for ( size_type i = 0; i < current_size; ++i )
            {
               p_temp[i] = std::move( p_alloc[i] );
            }

            p_allocator->deallocate( TO_BYTE_PTR( p_alloc ) );
            current_capacity = new_capacity;
         }
      }
      /**
       * @brief Return the current capacity of the memory allocation.
       *
       * @return The current capacity of the memory allocation.
       */
      constexpr size_type capacity( ) const noexcept { return current_capacity; };

      /**
       * @brief Remove all elements in the container.
       */
      void clear( ) noexcept
      {
         if constexpr ( !pod_type<value_type> )
         {
            for ( size_type i = 0; i < current_size; ++i )
            {
               p_alloc[i].~type_( );
            }
         }

         current_size = 0;
      }
      iterator insert( const_iterator pos, const value_type& value ) requires std::copyable<value_type>
      {
         size_type const new_size = current_size + 1;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size++ == 0 )
         {
            p_alloc[0] = value;

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[( &( *pos ) - p_alloc )] };
            for ( auto last = end( ) - 1; last != beg; --last )
            {
               std::iter_swap( last, last - 1 );
            }

            *beg = value;

            return beg;
         }
      }
      iterator insert( const_iterator pos, value_type&& value ) requires std::movable<value_type>
      {
         size_type const new_size = current_size + 1;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size++ == 0 )
         {
            p_alloc[0] = std::move( value );

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[( &( *pos ) - p_alloc )] };
            for ( auto last = end( ) - 1; last != beg; --last )
            {
               std::iter_swap( last, last - 1 );
            }

            *beg = std::move( value );

            return beg;
         }
      }
      iterator insert( const_iterator pos, size_type count, const value_type& value ) requires std::copyable<value_type>
      {
         size_type const new_size = current_size + count;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size == 0 )
         {
            for ( size_type i = 0; i < new_size; ++i )
            {
               p_alloc[i] = value;
            }

            current_size = new_size;

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[&( *pos ) - p_alloc] };

            current_size = new_size;

            for ( auto last = end( ) - count - 1; last >= beg; --last )
            {
               std::iter_swap( last, last + count );
            }

            for ( auto last = beg + count - 1; last >= beg; --last )
            {
               *last = value;
            }

            return beg;
         }
      }
      iterator insert( const_iterator pos, std::input_iterator auto first,
         std::input_iterator auto last ) requires std::copyable<value_type>
      {
         size_type const count = last - first;
         size_type const new_size = current_size + count;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size == 0 )
         {
            for ( size_type i = 0; first != last; ++first, ++i )
            {
               p_alloc[i] = *first;
            }

            current_size = new_size;

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[&( *pos ) - p_alloc] };

            current_size = new_size;

            for ( auto last = end( ) - count - 1; last >= beg; --last )
            {
               std::iter_swap( last, last + count );
            }

            for ( auto last = beg + count - 1; last >= beg; --last, ++first )
            {
               *last = *first;
            }

            return beg;
         }
      }
      iterator insert( const_iterator pos, std::initializer_list<value_type> init ) requires std::copyable<value_type>
      {
         size_type const new_size = current_size + init.size( );

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size == 0 )
         {
            current_size = new_size;

            for ( size_type i = 0; auto& it : init )
            {
               p_alloc[i++] = it;
            }

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[&( *pos ) - p_alloc] };

            current_size = new_size;

            for ( auto last = end( ) - init.size( ) - 1; last >= beg; --last )
            {
               std::iter_swap( last, last + init.size( ) );
            }

            auto last = beg + init.size( ) - 1;
            auto init_l = init.end( ) - 1;
            for ( ; last >= beg; --last, --init_l )
            {
               *last = *init_l;
            }

            return beg;
         }
      }
      template <class... args_>
      iterator emplace( const_iterator pos, args_&&... args ) requires std::constructible_from<value_type, args_...>
      {
         size_type const new_size = current_size + 1;

         assert( new_size < max_size( ) );

         reallocate( new_size );

         if ( current_size++ == 0 )
         {
            new ( TO_BYTE_PTR( p_alloc ) ) value_type( args... );

            return begin( );
         }
         else
         {
            auto beg = iterator{ &p_alloc[( &( *pos ) - p_alloc )] };
            for ( auto last = end( ) - 1; last != beg; --last )
            {
               std::iter_swap( last, last - 1 );
            }

            new ( TO_TYPE_PTR( &( *beg ) ) ) value_type( args... );

            return beg;
         }
      }

      iterator erase( const_iterator pos ) noexcept
      {
         if ( pos == cend( ) )
         {
            return end( );
         }
         else
         {
            auto it = iterator{ &p_alloc[( &( *pos ) - p_alloc )] };
            for ( auto curr = it; curr != end( ) - 1; ++curr )
            {
               std::iter_swap( curr, curr + 1 );
            }

            if constexpr ( pod_type<value_type> )
            {
               ( end( ) - 1 )->~value_type( );
            }

            --current_size;

            return it;
         }
      }
      iterator erase( const_iterator first, const_iterator last ) noexcept
      {
         size_type const count = std::distance( first, last );

         auto it = iterator{ &p_alloc[( &( *first ) - p_alloc )] };
         for ( auto curr = it; curr + count != end( ); ++curr )
         {
            std::iter_swap( curr, curr + count );
         }

         if constexpr ( !pod_type<value_type> )
         {
            for ( auto beg = end( ) - count; beg != end( ); ++beg )
            {
               beg->~value_type( );
            }
         }

         current_size = current_size - count;

         return it;
      }

      /**
       * @brief Copy a value at the end of the container.
       *
       * @details Copy the #value_type value at the end of the container. The container may need to reallocate the
       * memory and move all elements into the new memory allocation. In case of reallocation, all iterators to the
       * container will be invalidated. This function may only be used if the type
       * #value_type satisfies the <a href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a>
       * requirement.
       *
       * @param[in]  value    The value to move at the end of the container.
       */
      void push_back( const_reference value ) requires std::copyable<value_type>
      {
         assert( current_size + 1 < max_size( ) );

         reallocate( current_size + 1 );

         p_alloc[current_size++] = value;
      }
      /**
       * @brief Move an rvalue reference at the end of the container.
       *
       * @details Move the #value_type value at the end of the container. The container may need to reallocate the
       * memory and move all elements into the new memory allocation. In case of reallocation, all iterators to the
       * container will be invalidated. This function may only be used if the type
       * #value_type satisfies the <a href="https://en.cppreference.com/w/cpp/concepts/movable">std::movable</a>
       * requirement.
       *
       * @param[in]  value    The value to move at the end of the container.
       */
      void push_back( value_type&& value ) requires std::movable<value_type>
      {
         assert( current_size + 1 < max_size( ) );

         reallocate( current_size + 1 );

         p_alloc[current_size++] = std::move( value );
      }

      /**
       * @brief Construct an element at the end of the container.
       *
       * @tparam  args_    The type of the arguments needed to construct the new element.
       * @param   args     The arguments needed to construct the new element.
       *
       * @return A reference to the newly constructed element at the end of the container.
       */
      template <class... args_>
      reference emplace_back( args_&&... args ) requires std::constructible_from<value_type, args_...>
      {
         assert( current_size + 1 < max_size( ) );

         reallocate( current_size + 1 );

         new ( TO_BYTE_PTR( p_alloc + current_size ) ) value_type( args... );

         return p_alloc[current_size++];
      }

      /**
       * @brief Remove the last element in the container.
       */
      void pop_back( )
      {
         if constexpr ( !pod_type<value_type> )
         {
            ( end( ) - 1 )->~value_type( );
         }

         --current_size;
      }

      void resize( size_type count ) requires std::default_initializable<value_type>
      {
         assert( count != 0 );
         assert( count <= max_size( ) );

         if ( count < current_size )
         {
            if constexpr ( !pod_type<value_type> )
            {
               for ( int i = count; i < current_size; ++i )
               {
                  p_alloc[i].~value_type( );
               }
            }

            current_size = count;
         }
         else if ( count > current_size )
         {
            reallocate( count );

            for ( int i = current_size; i < count; ++i )
            {
               new ( TO_BYTE_PTR( p_alloc + i ) ) value_type( );
            }

            current_size = count;
         }
      }
      void resize( size_type count, const_reference value ) requires std::copyable<value_type>
      {
         assert( count != 0 );
         assert( count <= max_size( ) );

         if ( count < current_size )
         {
            if constexpr ( !pod_type<value_type> )
            {
               for ( int i = count; i < current_size; ++i )
               {
                  p_alloc[i].~value_type( );
               }
            }

            current_size = count;
         }
         else if ( count > current_size )
         {
            reallocate( count );

            for ( int i = current_size; i < count; ++i )
            {
               new ( TO_BYTE_PTR( p_alloc + i ) ) value_type( value );
            }

            current_size = count;
         }
      }

   private:
      void reallocate( size_type new_size )
      {
         if ( !p_alloc )
         {
            p_alloc = TO_TYPE_PTR( p_allocator->allocate( sizeof( value_type ) * new_size, alignof( value_type ) ) );
            if ( !p_alloc )
            {
               throw std::bad_alloc{ };
            }

            current_capacity = new_size;
         }
         else if ( new_size > current_capacity )
         {
            current_capacity = p_allocator->allocation_capacity( TO_BYTE_PTR( p_alloc ) ) / sizeof( value_type );
            if ( new_size > current_capacity )
            {
               pointer p_temp =
                  TO_TYPE_PTR( p_allocator->allocate( sizeof( value_type ) * new_size, alignof( value_type ) ) );
               if ( !p_temp )
               {
                  throw std::bad_alloc{ };
               }

               for ( size_type i = 0; i < current_size; ++i )
               {
                  if constexpr ( std::movable<value_type> )
                  {
                     p_temp[i] = std::move( p_alloc[i] );
                  }
                  else
                  {
                     p_temp[i] = p_alloc[i];
                  }

                  if constexpr ( pod_type<value_type> )
                  {
                     p_alloc[i].~value_type( );
                  }
               }

               p_alloc = p_temp;
               current_capacity = new_size;
            }
         }
      }

   private:
      allocator_type* p_allocator{ nullptr };

      pointer p_alloc{ nullptr };
      size_type current_capacity{ 0 };
      size_type current_size{ 0 };
   }; // namespace ESL

#undef TO_TYPE_PTR
} // namespace ESL
