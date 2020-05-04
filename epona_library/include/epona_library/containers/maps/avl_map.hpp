/**
 * @file avl_map.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Tuesday, April 21st, 2020
 * @brief Contains all data containers implemented using an <a href="https://en.wikipedia.org/wiki/AVL_tree">AVL
 * tree</a>.
 *
 * @copyright MIT License
 */

#pragma once

#include <epona_library/allocators/multipool_allocator.hpp>
#include <epona_library/utils/concepts.hpp>

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <utility>

namespace ESL
{
   /**
    * @class avl_map avl_map.hpp <ESL/containers/maps/avl_map.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date
    *
    * @brief An ordered data structure implemented an <a href="https://en.wikipedia.org/wiki/AVL_tree">AVL tree</a>
    * using nodes in a linked-list fashion.
    *
    * @tparam key_ The type of the key. Must satisfy the <a
    * href="https://en.cppreference.com/w/cpp/concepts/equality_comparable">std::equality_comparable</a> requirement.
    * @tparam any_ The type of the value to store along the key.
    * @tparam allocator_ The type of the allocator. Must satisfy the ESL::basic_allocator requirement.
    * @tparam compare_ The comparison functor to use when comparing keys.
    */
   template <std::equality_comparable key_, class any_, basic_allocator allocator_ = multipool_allocator,
      class compare_ = std::less<key_>>
   class avl_map
   {
      struct node
      {
         using key_type = key_;
         using mapped_type = any_;
         using value_type = std::pair<key_type, mapped_type>;
         using self_type = node;
         using pointer = self_type*;

         value_type data{ };
         pointer p_parent{ nullptr };
         pointer p_left{ nullptr };
         pointer p_right{ nullptr };
         std::int8_t height{ -1 };
      };

      template <class value_>
      class inorder_iterator
      {
      public:
         using iterator_category = std::bidirectional_iterator_tag;
         using self_type = inorder_iterator;
         using value_type = value_;
         using reference = value_type&;
         using const_reference = value_type const&;
         using pointer = value_type*;
         using const_pointer = value_type const*;
         using difference_type = std::ptrdiff_t;

         constexpr inorder_iterator( ) noexcept = default;
         constexpr explicit inorder_iterator( node* p_elem ) noexcept : p_type( p_elem ) {}

         constexpr bool operator==( self_type const& rhs ) const noexcept = default;

         constexpr reference operator*( ) noexcept
         {
            assert( p_type != nullptr && "Cannot derefence a nullptr" );

            return p_type->data;
         }
         constexpr const_reference operator*( ) const noexcept
         {
            assert( p_type != nullptr && "Cannot derefence a nullptr" );

            return p_type->data;
         }
         constexpr pointer operator->( ) noexcept
         {
            assert( p_type != nullptr && "Cannot use derefence a nullptr" );

            return &p_type->data;
         }
         constexpr const_pointer operator->( ) const noexcept
         {
            assert( p_type != nullptr && "Cannot derefence a nullptr" );

            return &p_type->data;
         };

         constexpr self_type& operator++( ) noexcept
         {
            if ( p_type->p_right )
            {
               p_type = p_type->p_right;
               while ( p_type->p_left )
               {
                  p_type = p_type->p_left;
               }
            }
            else
            {
               node const* p_temp{ nullptr };
               do
               {
                  p_temp = p_type;
                  p_type = p_type->p_parent;
               } while ( p_type && p_temp == p_type->p_right );
            }

            return *this;
         }
         constexpr self_type& operator--( ) noexcept
         {
            if ( p_type->p_left )
            {
               p_type = p_type->p_left;
               while ( p_type->p_right )
               {
                  p_type = p_type->p_right;
               }
            }
            else
            {
               node const* p_temp;
               do
               {
                  p_temp = p_type;
                  p_type = p_type->p_parent;
               } while ( p_type && p_temp == p_type->p_left );
            }

            return *this;
         }

         constexpr self_type operator++( int ) const noexcept
         {
            self_type it = *this;
            ++( *this );

            return it;
         }
         constexpr self_type operator--( int ) const noexcept
         {
            self_type it = *this;
            --*this;

            return it;
         }

         constexpr void swap( self_type& rhs ) noexcept { std::swap( p_type, rhs.p_type ); }

      private:
         node* p_type{ nullptr };
      };

   public:
      using key_type = typename node::key_type;
      using mapped_type = typename node::mapped_type;
      using value_type = typename node::value_type;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using key_compare = compare_;
      using reference = value_type&;
      using const_reference = value_type const&;
      using pointer = value_type*;
      using const_pointer = value_type const*;
      using iterator = inorder_iterator<value_type>;
      using const_iterator = inorder_iterator<value_type const>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      explicit avl_map( allocator_* const p_allocator ) noexcept : p_allocator( p_allocator ) {}
      avl_map( std::input_iterator auto first, std::input_iterator auto last, allocator_* const p_allocator ) :
         p_allocator( p_allocator )
      {}
      avl_map( std::initializer_list<value_type> init, allocator_* const p_allocator ) : p_allocator( p_allocator ) {}
      avl_map( avl_map const& other ) {}
      avl_map( avl_map const& other, allocator_* const p_allocator ) {}
      avl_map( avl_map&& other ) noexcept {}
      avl_map( avl_map&& other, allocator_* const p_allocator ) noexcept {}
      ~avl_map( )
      {
         clear( );

         count = 0;
      }

      /**
       * @brief Return a pointer to the container's current allocator.
       *
       * @return A pointer to the container's current allocator.
       */
      allocator_* const get_allocator( ) noexcept { return p_allocator; }
      /**
       * @brief Return a pointer to the container's current allocator.
       *
       * @return A pointer to the container's current allocator.
       */
      allocator_ const* const get_allocator( ) const noexcept { return p_allocator; }

      /**
       * @brief Return an #iterator to the left-most #value_type in the tree
       *
       * @return The #iterator to the left-most #value_type.
       */
      iterator begin( ) noexcept
      {
         node* p_node = p_root;
         while ( p_node->p_left )
         {
            p_node = p_node->p_left;
         }

         return iterator{ p_node };
      }

      /**
       * @brief Return an #const_iterator to the left-most #value_type in the tree
       *
       * @return The #const_iterator to the left-most #value_type.
       */
      const_iterator begin( ) const noexcept
      {
         node* p_node = p_root;
         while ( p_node->p_left )
         {
            p_node = p_node->p_left;
         }

         return const_iterator{ p_node };
      }

      /**
       * @brief Return an #const_iterator to the left-most #value_type in the tree
       *
       * @return The #const_iterator to the left-most #value_type.
       */
      const_iterator cbegin( ) const noexcept
      {
         node* p_node = p_root;
         while ( p_node->p_left )
         {
            p_node = p_node->p_left;
         }

         return const_iterator{ p_node };
      }

      /**
       * @brief Return an #iterator to one past the last element in the container.
       *
       * @return The #iterator to one past the last element.
       */
      iterator end( ) noexcept { return iterator{ p_root->p_parent }; }

      /**
       * @brief Return an #const_iterator to one past the last element in the container.
       *
       * @return The #const_iterator to one past the last element.
       */
      const_iterator end( ) const noexcept { return const_iterator{ p_root->p_parent }; }

      /**
       * @brief Return an #const_iterator to one past the last element in the container.
       *
       * @return The #const_iterator to one past the last element.
       */
      const_iterator cend( ) const noexcept { return const_iterator{ p_root->p_parent }; }

      /**
       * @brief Return a #reverse_iterator to the left-most #value_type in the container.
       *
       * @return A #reverse_iterator to the left-most #value_type in the container.
       */
      reverse_iterator rbegin( ) noexcept
      {
         node* p_node = p_root;
         while ( p_node->p_left )
         {
            p_node = p_node->p_left;
         }

         return { p_node };
      }

      /**
       * @brief Return a #const_reverse_iterator to the left-most #value_type in the container.
       *
       * @return A #const_reverse_iterator to the left-most #value_type in the container.
       */
      const_reverse_iterator rbegin( ) const noexcept
      {
         node* p_node = p_root;
         while ( p_node->p_left )
         {
            p_node = p_node->p_left;
         }

         return { p_node };
      }

      /**
       * @brief Return a #const_reverse_iterator to the left-most #value_type in the container.
       *
       * @return A #const_reverse_iterator to the left-most #value_type in the container.
       */
      const_reverse_iterator crbegin( ) const noexcept
      {
         node* p_node = p_root;
         while ( p_node->p_left )
         {
            p_node = p_node->p_left;
         }

         return { p_node };
      }

      /**
       * @brief Check if the container is empty or not.
       *
       * @return True if the container is empty, otherwise false.
       */
      [[nodiscard]] bool empty( ) const noexcept { return count == 0; }

      /**
       * @brief Return the number of elements in the container;
       *
       * @return The number of elements in the container.
       */
      size_type size( ) const noexcept { return count; }

      /**
       * @brief Return the maximum number of elements the container can support.
       *
       * @return The maximum number of elements the container can support.
       */
      size_type max_size( ) const noexcept { return std::numeric_limits<difference_type>::max( ); }

      /**
       * @brief Remove all elements from the container
       */
      void clear( ) noexcept
      {
         if ( p_root )
         {
            clear_util( p_root );

            p_root->~node( );
            p_allocator->deallocate( reinterpret_cast<std::byte*>( p_root ) );
         }

         p_root = nullptr;
         count = 0;
      }

      /**
       * @brief Insert a #value_type into the container.
       *
       * @details Insert a new #value_type in the container. If the insertion is successful, an iterator to the newly
       * inserted value will be returned along with a boolean value set to true. If the key contained in the
       * #value_type is already present, the #value_type will not be inserted in the container, in which case the
       * boolean return value will be false. This function may only be used if the type #value_type satisfies the
       * <a href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a> requirement. Iterators
       * may be invalidated due to rebalencing of the container.
       *
       * @throw std::bad_alloc Thrown if the allocator fails to allocate memory. If the exception occurs, the
       * state of the container will remain unchanged and valid.
       *
       * @param[in] value The ##value_type to insert into the container
       *
       * @return An interator to the element and a boolean to say if the insertion took place.
       */
      std::pair<iterator, bool> insert( const_reference value ) requires std::copyable<value_type>
      {
         auto [p_node, p_new, took_place] = find_insert( value, p_root );

         p_root = p_node;

         return { iterator{ p_new }, took_place };
      }

      /**
       * @brief Insert an rvalue #value_type into the container.
       *
       * @details Insert an rvalue #value_type in the container. If the insertion was successful, an iterator to the
       * newly inserted value will be returned along with a boolean value set to true. If the key contained in the
       * #value_type is already present, the #value_type will not be inserted in the container, in which case the
       * boolean return value will be false. This function may only be used if the type #value_type satisfies the
       * <a href="https://en.cppreference.com/w/cpp/concepts/copyable">std::movable</a> requirement. Iterators
       * may be invalidated due to rebalencing of the container.
       *
       * @throw std::bad_alloc Thrown if the allocator fails to allocate memory. If the exception occurs, the
       * state of the container will remain unchanged and valid.
       *
       * @param[in] value The #value_type to insert into the container
       *
       * @return An interator to the element and a boolean to say if the insertion took place.
       */
      std::pair<iterator, bool> insert( value_type&& value ) requires std::movable<value_type>
      {
         auto [p_start, p_new, took_place] = insert_util( std::move( value ), p_root );

         p_root = p_start;

         return { iterator{ p_new }, took_place };
      }

      template <class value_>
      std::pair<iterator, bool> insert( value_&& value ) requires std::constructible_from<value_type, value_>
      {
         return emplace( std::forward<value_>( value ) );
      }

      /**
       * @brief Inserts elements from range [first, last).
       *
       * @details Inserts elements from range [first, last). If multiple elements have the same #key_type value, only
       * the first will be inserted. Iterators may be invalidated due to rebalencing of the container.
       *
       * @throw std::bad_alloc Thrown if the allocator fails to allocate memory. If the exception occurs, the
       * state of the container will remain unchanged and valid.
       *
       * @param[in] first An iterator to the first element to insert inclusive.
       * @param[in] last An iterator to the last element to insert exclusive.
       */
      void insert( std::input_iterator auto first, std::input_iterator auto last )
      {
         for ( ; first != last; ++first )
         {
            if constexpr ( std::movable<value_type> )
            {
               insert( std::move( *first ) );
            }
            else
            {
               insert( *first );
            }
         }
      }

      /**
       * @brief Insert elements from an <a
       * href="https://en.cppreference.com/w/cpp/utility/initializer_list">std::initializer_list</a> in the container.
       *
       * @details Insert elements from an <a
       * href="https://en.cppreference.com/w/cpp/utility/initializer_list">std::initializer_list</a> in the container.
       * If multiple elements have the same #key_type value., only the first will be inserted into the container.
       *
       * @throw std::bad_alloc Thrown if the allocator fails to allocate memory. If the exception occurs, the
       * state of the container will remain unchanged and valid.
       *
       * @param[in] initializer_list A list of #value_type to insert in the container.
       */
      void insert( std::initializer_list<value_type> initializer_list )
      {
         for ( const_reference it : initializer_list )
         {
            if constexpr ( std::movable<value_type> )
            {
               insert( std::move( it ) );
            }
            else
            {
               insert( it );
            }
         }
      }

      /**
       * @brief Insert a new element into the container constructed in-place with the given args.
       *
       * @details Insert a new element constructed in-place with the given. If the insertion was successful, an
       * iterator to the newly inserted value will be returned along with a boolean value set to true. If the key
       * contained in the #value_type is already present, the #value_type will not be inserted in the container, in
       * which case the boolean return value will be false. Iterators may be invalidated due to rebalencing of the
       * container.
       *
       * @throw std::bad_alloc Thrown if the allocator fails to allocate memory. If the exception occurs, the
       * state of the container will remain unchanged and valid.
       *
       * @tparam The argument types used for the construction of an object of type #value_type.
       *
       * @param[in] args The arguments used for the in-place construction of the object.
       *
       * @return An interator to the element and a boolean to say if the insertion took place.
       */
      template <class... args_>
      std::pair<iterator, bool> emplace( args_&&... args ) requires std::constructible_from<value_type, args_...>
      {
         auto [p_node, p_new, took_place] = insert_util( value_type( args... ), p_root );

         p_root = p_node;

         return { iterator{ p_new }, took_place };
      }

      iterator find( key_type const& key ) { return { find_util( key, p_root ) }; }
      const_iterator find( key_type const& key ) const { return { find_util( key, p_root ) }; }

   private:
      void clear_util( node* p_node )
      {
         if ( p_node->p_left )
         {
            clear_util( p_node->p_left );

            p_node->p_left->data.~value_type( );

            p_allocator->deallocate( reinterpret_cast<std::byte*>( p_node->p_left ) );
            p_node->p_left = nullptr;
         }

         if ( p_node->p_right )
         {
            clear_util( p_node->p_right );

            p_node->p_right->~node( );

            p_allocator->deallocate( reinterpret_cast<std::byte*>( p_node->p_right ) );
            p_node->p_right = nullptr;
         }
      }

      std::tuple<node*, node*, bool> insert_util( const_reference value, node* p_start )
      {
         if ( !p_start )
         {
            void* p_memory = p_allocator->allocate( sizeof( node ), alignof( node ) );
            if ( !p_memory )
            {
               throw std::bad_alloc{ };
            }

            p_start = new ( p_memory ) node( { .data = value, .height = 0 } );

            ++count;

            return { p_start, p_start, true };
         }

         if ( key_compare comp{ }; comp( value.first, p_start->data.first ) )
         {
            auto [p_node, p_new, took_place] = insert_util( std::move( value ), p_start->p_left );
            if ( took_place )
            {
               p_start->p_left = p_node;
               p_node->p_parent = p_start;

               if ( height( p_start->p_left ) - height( p_start->p_right ) == 2 )
               {
                  if ( comp( p_new->data.first, p_node->data.first ) )
                  {
                     p_start = right_rotate( p_start );
                  }
                  else
                  {
                     p_start->p_left = left_rotate( p_start->p_left );
                     p_start = right_rotate( p_start );
                  }
               }

               p_start->height = std::max( height( p_start->p_left ), height( p_start->p_right ) ) + 1;
            }

            return { p_start, p_new, took_place };
         }
         else if ( value.first == p_start->data.first )
         {
            return { p_start, p_start, false };
         }
         else
         {
            auto [p_node, p_new, took_place] = insert_util( std::move( value ), p_start->p_right );
            if ( took_place )
            {
               p_start->p_right = p_node;
               p_node->p_parent = p_start;

               if ( height( p_start->p_right ) - height( p_start->p_left ) == 2 )
               {
                  if ( !comp( p_new->data.first, p_node->data.first ) )
                  {
                     p_start = left_rotate( p_start );
                  }
                  else
                  {
                     p_start->p_right = right_rotate( p_start->p_right );
                     p_start = left_rotate( p_start );
                  }
               }

               p_start->height = std::max( height( p_start->p_left ), height( p_start->p_right ) ) + 1;
            }

            return { p_start, p_new, took_place };
         }
      }

      std::tuple<node*, node*, bool> insert_util( value_type&& value, node* p_start )
      {
         if ( !p_start )
         {
            void* p_memory = p_allocator->allocate( sizeof( node ), alignof( node ) );
            if ( !p_memory )
            {
               throw std::bad_alloc{ };
            }

            p_start = new ( p_memory ) node( { .data = std::move( value ), .height = 0 } );

            ++count;

            return { p_start, p_start, true };
         }

         if ( key_compare comp{ }; comp( value.first, p_start->data.first ) )
         {
            auto [p_node, p_new, took_place] = insert_util( std::move( value ), p_start->p_left );
            if ( took_place )
            {
               p_start->p_left = p_node;
               p_node->p_parent = p_start;

               if ( height( p_start->p_left ) - height( p_start->p_right ) == 2 )
               {
                  if ( comp( p_new->data.first, p_node->data.first ) )
                  {
                     p_start = right_rotate( p_start );
                  }
                  else
                  {
                     p_start->p_left = left_rotate( p_start->p_left );
                     p_start = right_rotate( p_start );
                  }
               }

               p_start->height = std::max( height( p_start->p_left ), height( p_start->p_right ) ) + 1;
            }

            return { p_start, p_new, took_place };
         }
         else if ( value.first == p_start->data.first )
         {
            return { p_start, p_start, false };
         }
         else
         {
            auto [p_node, p_new, took_place] = insert_util( std::move( value ), p_start->p_right );
            if ( took_place )
            {
               p_start->p_right = p_node;
               p_node->p_parent = p_start;

               if ( height( p_start->p_right ) - height( p_start->p_left ) == 2 )
               {
                  if ( !comp( p_new->data.first, p_node->data.first ) )
                  {
                     p_start = left_rotate( p_start );
                  }
                  else
                  {
                     p_start->p_right = right_rotate( p_start->p_right );
                     p_start = left_rotate( p_start );
                  }
               }

               p_start->height = std::max( height( p_start->p_left ), height( p_start->p_right ) ) + 1;
            }

            return { p_start, p_new, took_place };
         }
      }

      node* find_util( key_type const& key, node* p_node )
      {
         if ( !p_node )
         {
            return nullptr;
         }

         if ( key == p_node->data.first )
         {
            return p_node;
         }

         if ( key_compare comp{ }; comp( key, p_node->data.first ) )
         {
            return find_util( key, p_node->p_left );
         }
         else
         {
            return find_util( key, p_node->p_right );
         }
      }

      std::int8_t height( node const* const p_node ) const noexcept { return p_node == nullptr ? -1 : p_node->height; }

      node* left_rotate( node* const p_node ) noexcept
      {
         node* const p_temp = p_node->p_right;
         p_node->p_right = p_temp->p_left;

         if ( p_temp->p_left )
         {
            p_temp->p_left->p_parent = p_node;
         }

         p_temp->p_parent = p_node->p_parent;

         if ( !p_node->p_parent )
         {
            p_root = p_temp;
         }
         else if ( p_node == p_node->p_parent->p_left )
         {
            p_node->p_parent->p_left = p_temp;
         }
         else
         {
            p_node->p_parent->p_right = p_temp;
         }

         p_temp->p_left = p_node;
         p_node->p_parent = p_temp;

         p_node->height = std::max( height( p_node->p_left ), height( p_node->p_right ) );
         p_temp->height = std::max( height( p_temp->p_left ), height( p_temp->p_right ) );

         return p_temp;
      }

      node* right_rotate( node* const p_node ) noexcept
      {
         node* const p_temp = p_node->p_left;
         p_node->p_left = p_temp->p_right;

         if ( p_temp->p_right )
         {
            p_temp->p_right->p_parent = p_node;
         }

         p_node->p_parent = p_temp->p_parent;

         if ( !p_node->p_parent )
         {
            p_root = p_temp;
         }
         else if ( p_node == p_node->p_parent->p_right )
         {
            p_node->p_parent->p_right = p_temp;
         }
         else
         {
            p_node->p_parent->p_left = p_temp;
         }

         p_temp->p_right = p_node;
         p_node->p_parent = p_temp;

         p_node->height = std::max( height( p_node->p_left ), height( p_node->p_right ) );
         p_temp->height = std::max( height( p_temp->p_left ), height( p_temp->p_right ) );

         return p_temp;
      }

   private:
      allocator_* p_allocator{ nullptr };
      node* p_root{ nullptr };

      size_type count{ 0 };
   }; // namespace ESL
} // namespace ESL
