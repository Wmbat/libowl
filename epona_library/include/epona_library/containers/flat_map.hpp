/**
 * @file flat_map.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 6th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <epona_library/allocators/allocator_utils.hpp>
#include <epona_library/allocators/multipool_allocator.hpp>
#include <epona_library/containers/details.hpp>
#include <epona_library/utils/concepts.hpp>
#include <epona_library/utils/error_handling.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>

namespace ESL
{
   template <full_allocator allocator_>
   class tiny_flat_map_base
   {
   public:
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = allocator_;
      using pointer = void*;

   protected:
      tiny_flat_map_base() = delete;
      tiny_flat_map_base(pointer p_first_element, size_type capacity, allocator_type* p_alloc) :
         p_begin(p_first_element), p_alloc(p_alloc), cap(capacity)
      {}

      /**
       * @brief Return the container's capacity;
       *
       * @return The container's current memory capacity.
       */
      size_type capacity() const noexcept { return cap; }

      /**
       * @brief A special function to grow the container's memory with trivial types.
       *
       * @param[in] p_first_element A pointer to the first element in the container.
       * @param[in] min_cap The maximum capacity to grow the container by.
       * @param[in] type_size The size of the trivial type.
       * @param[in] type_align The alignment of the trivial type.
       */
      void grow_trivial(void* p_first_element, size_type min_cap, size_type type_size, size_type type_align)
      {
         size_type const new_capacity = std::clamp(2 * capacity() + 1, min_cap, std::numeric_limits<size_type>::max());

         if (p_begin == p_first_element)
         {
            void* p_new = p_alloc->allocate(new_capacity * type_size, type_align);
            if (!p_new)
            {
               handle_bad_alloc_error("tiny flat map capacity overflow during reallocation.");
            }

            memcpy(p_new, p_begin, size() * type_size);

            p_begin = p_new;
         }
         else
         {
            void* p_new = p_alloc->reallocate(p_begin, new_capacity * type_size);
            if (!p_new)
            {
               handle_bad_alloc_error("tiny flat map capacity overflow during reallocation.");
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
      allocator_type* get_allocator() noexcept { return p_alloc; }
      /**
       * @brief Return a pointer to the container's allocator.
       *
       * @return The pointer to the container's allocator
       */
      allocator_type const* get_allocator() const noexcept { return p_alloc; }

      /**
       * @brief Check if the container has no element.
       *
       * @return True if the container is empty, otherwise false.
       */
      [[nodiscard]] constexpr bool empty() const noexcept { return count == 0; }
      /**
       * @brief Return the number of elements in the container.
       *
       * @return The size of the container.
       */
      size_type size() const noexcept { return count; }
      /**
       * @brief Return the maximum size of the container.
       *
       * @return The maximum value held by the #difference_type.
       */
      size_type max_size() const noexcept { return std::numeric_limits<difference_type>::max(); }

   protected:
      pointer p_begin{nullptr};
      allocator_type* p_alloc{nullptr};
      size_type count{0};
      size_type cap{0};
   };

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
   template <class pair_, complex_allocator<std::optional<pair_>> allocator_>
   struct tiny_flat_map_align_and_size
   {
      std::aligned_storage_t<sizeof(tiny_flat_map_base<allocator_>), alignof(tiny_flat_map_base<allocator_>)> base;
      std::aligned_storage_t<sizeof(std::size_t), alignof(std::size_t)> padding;
      std::aligned_storage_t<sizeof(std::optional<pair_>), alignof(std::optional<pair_>)> first_element;
   };

   namespace details
   {
      template <class key_, class val_>
      using opt_pair = std::optional<std::pair<key_, val_>>;
   }

   template <std::equality_comparable key_, class val_, complex_allocator<details::opt_pair<key_, val_>> allocator_,
      class compare_ = std::less<key_>>
   class tiny_flat_avl_map_impl : public tiny_flat_map_base<allocator_>
   {
      using super = tiny_flat_map_base<allocator_>;

      template <class value_>
      class inorder_iterator
      {
         using outer = tiny_flat_avl_map_impl;

      public:
         using iterator_category = std::bidirectional_iterator_tag;
         using self_type = inorder_iterator;
         using value_type = value_;
         using reference = value_type&;
         using const_reference = value_type const&;
         using pointer = value_type*;
         using const_pointer = value_type const*;
         using difference_type = std::ptrdiff_t;

         constexpr inorder_iterator() noexcept = default;
         constexpr explicit inorder_iterator(std::size_t index) noexcept : index(index) {}

         constexpr bool operator==(self_type const& rhs) const noexcept = default;

         constexpr reference operator*() noexcept
         {
            auto* p_data = static_cast<std::optional<value_type>*>(outer::p_begin);

            return p_data[index].value;
         }
         constexpr const_reference operator*() const noexcept
         {
            auto* p_data = static_cast<std::optional<value_type>*>(outer::p_begin);

            return p_data[index].value;
         }
         constexpr pointer operator->() noexcept
         {
            auto* p_data = static_cast<std::optional<value_type>*>(outer::p_begin);

            return &p_data[index].value;
         }
         constexpr const_pointer operator->() const noexcept
         {
            auto* p_data = static_cast<std::optional<value_type>*>(outer::p_begin);

            return &p_data[index].value;
         }

         constexpr self_type& operator++() noexcept
         {
            auto* p_data = static_cast<std::optional<value_type>*>(outer::p_begin);

            std::size_t const right = outer::find_right(index);
            if (right < outer::capacity() && p_data[right].has_value())
            {
               index = right;

               std::size_t const left = outer::find_left(index);
               while (left < outer::capacity() && p_data[left].has_value())
               {
                  index = left;
               }
            }
            else
            {
               std::size_t temp = 0;
               do
               {
                  temp = index;
                  index = outer::find_parent(index);
               } while ((index < outer::capacity() && p_data[index]) && temp == outer::find_right(index));
            }
         }
         constexpr self_type& operator--() noexcept
         {
            auto* p_data = static_cast<std::optional<value_type>*>(outer::p_begin);

            std::size_t const left = outer::find_left(index);
            if (left < outer::capacity() && p_data[left].has_value())
            {
               index = left;

               std::size_t const right = outer::find_right(index);
               while (right < outer::capacity() && p_data[right].has_value())
               {
                  index = right;
               }
            }
            else
            {
               std::size_t temp = 0;
               do
               {
                  temp = index;
                  index = outer::find_parent(index);
               } while ((index < outer::capacity() && p_data[index]) && temp == outer::find_left(index));
            }
         }

         constexpr self_type operator++(int) const noexcept
         {
            self_type it = *this;
            ++(*this);

            return it;
         }
         constexpr self_type operator--(int) const noexcept
         {
            self_type it = *this;
            --(*this);

            return it;
         }

         constexpr void swap(self_type& rhs) noexcept { std::swap(index, rhs.index); }

      private:
         std::size_t index{std::numeric_limits<std::size_t>::max()};
      };

   public:
      using key_type = key_;
      using mapped_type = val_;
      using value_type = std::pair<key_type, mapped_type>;
      using size_type = typename super::size_type;
      using difference_type = typename super::difference_type;
      using key_compare = compare_;
      using allocator_type = typename super::allocator_type;
      using reference = value_type&;
      using const_reference = value_type const&;
      using pointer = value_type*;
      using const_pointer = value_type const*;
      using iterator = inorder_iterator<value_type>;
      using const_iterator = inorder_iterator<value_type const>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<iterator const>;

   private:
      using opt_type = std::optional<value_type>;

   protected:
      tiny_flat_avl_map_impl() = delete;
      tiny_flat_avl_map_impl(size_type capacity, allocator_type* p_allocator) :
         super(get_first_element(), capacity, p_allocator)
      {}

      /**
       * @brief Check if the container is currently using the static memory buffer.
       *
       * @return True if the container is using the static memory buffer, otherwise false.
       */
      bool is_static() const noexcept { return super::p_begin == get_first_element(); }

   public:
      iterator begin() noexcept
      {
         auto const* const p_data = static_cast<opt_type*>(super::p_begin);
         std::size_t left = find_left(0);

         while (left < super::capacity(), p_data[left].has_value())
         {
            left = find_left(left);
         }

         return iterator{left};
      }
      const_iterator begin() const noexcept
      {
         auto const* const p_data = static_cast<opt_type*>(super::p_begin);
         std::size_t left = find_left(0);

         while (left < super::capacity(), p_data[left].has_value())
         {
            left = find_left(left);
         }

         return iterator{left};
      }
      const_iterator cbegin() const noexcept
      {
         auto const* const p_data = static_cast<opt_type*>(super::p_begin);
         std::size_t left = find_left(0);

         while (left < super::capacity(), p_data[left].has_value())
         {
            left = find_left(left);
         }

         return iterator{left};
      }

      iterator end() noexcept { return iterator{std::numeric_limits<size_type>::max()}; }
      const_iterator end() const noexcept { return iterator{std::numeric_limits<size_type>::max()}; }
      const_iterator cend() const noexcept { return iterator{std::numeric_limits<size_type>::max()}; }

      std::pair<iterator, bool> insert(const_reference value) requires std::copyable<value_type>
      {
         auto* p_data = static_cast<opt_type*>(super::p_begin);

         if (p_data[0].has_value())
         {
         }
      }

   private:
      void* get_first_element() const noexcept
      {
         using layout = tiny_flat_map_align_and_size<value_type, allocator_type>;

         return const_cast<void*>(
            reinterpret_cast<void const*>(reinterpret_cast<char const*>(this) + offsetof(layout, first_element)));
      }

      void grow(size_type min_size = 0)
      {
         assert(super::p_alloc != nullptr);

         if (min_size > super::max_size())
         {
            handle_bad_alloc_error("tiny flat map capacity overflow during allocation.");
         }

         if constexpr (trivially_copyable<value_type>)
         {
            super::grow_trivial(get_first_element(), min_size, sizeof(opt_type), alignof(opt_type));
         }
         else
         {
            size_type new_cap = std::clamp(2 * super::capacity() + 1, min_size, std::numeric_limits<size_type>::max());

            if (opt_type* p_new = nullptr; is_static())
            {
               p_new = static_cast<opt_type*>(super::p_alloc->allocate(new_cap * sizeof(opt_type), alignof(opt_type)));
               if (!p_new)
               {
                  handle_bad_alloc_error("tiny flat map capacity overflow during allocation.");
               }
            }
            else
            {
               p_new = super::p_alloc->template reallocate<opt_type>(static_cast<opt_type*>(super::p_begin), new_cap);
               if (!p_new)
               {
                  handle_bad_alloc_error("tiny flat map capacity overflow during allocation.");
               }
            }
         }
      }

      static void destroy_range(iterator first, iterator last)
      {
         if constexpr (!trivial<value_type>)
         {
            while (first != last)
            {
               first->~value_type();
               ++first;
            }
         }
      }

      static constexpr size_type find_left(size_type index) noexcept { return 2 * index + 1; }
      static constexpr size_type find_right(size_type index) noexcept { return 2 * index + 2; }
      static constexpr size_type find_parent(size_type index) noexcept { return (index - 1) / 2; }
   };

   template <std::equality_comparable key_, class val_, std::size_t buff_sz,
      complex_allocator<details::opt_pair<key_, val_>> allocator_ = ESL::multipool_allocator,
      class compare_ = std::less<key_>>
   class tiny_flat_avl_map :
      public tiny_flat_avl_map_impl<key_, val_, allocator_, compare_>,
      details::static_array_storage<details::opt_pair<key_, val_>, buff_sz>
   {
      using super = tiny_flat_avl_map_impl<key_, val_, allocator_, compare_>;
      using storage = details::static_array_storage<details::opt_pair<key_, val_>, buff_sz>;

   public:
      using key_type = typename super::key_type;
      using mapped_type = typename super::mapped_type;
      using value_type = typename super::value_type;
      using size_type = typename super::size_type;
      using difference_type = typename super::difference_type;
      using key_compare = typename super::key_compare;
      using allocator_type = typename super::allocator_type;

   public:
      tiny_flat_avl_map(allocator_type* p_allocator) : super(buff_sz, p_allocator) {}
   };
} // namespace ESL
