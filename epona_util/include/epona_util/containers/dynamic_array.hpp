/**
 * @file dynamic_array.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, April 23rd, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <epona_util/compare.hpp>
#include <epona_util/concepts.hpp>
#include <epona_util/containers/details.hpp>
#include <epona_util/containers/error_handling.hpp>
#include <epona_util/iterators/input_iterator.hpp>
#include <epona_util/iterators/random_access_iterator.hpp>
#include <epona_util/monad/either.hpp>

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace util
{
   template <class any_, std::size_t buffer_size_>
   class test
   {
   public:
      using value_type = any_;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using iterator = random_access_iterator<value_type>;
      using const_iterator = random_access_iterator<const value_type>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      explicit test() : m_capacity{buffer_size_} {}
      explicit test(size_type count) requires std::default_initializable<value_type> :
         m_capacity{buffer_size_}
      {
         if (!assign(count, value_type{}))
         {
            handle_bad_alloc_error("Failed to allocate memory for dynamic array creation");
         }
      }

      void assign(size_type count, const_reference value) noexcept
         requires std::copyable<value_type>
      {
         clear();

         if (count > m_capacity)
         {
            grow(count);
         }

         m_size += count;

         std::uninitialized_fill(begin(), end(), value);
      }

      auto begin() noexcept -> iterator
      {
         if (m_data)
         {
            return iterator{m_data.get()};
         }
         else
         {
            return iterator{reinterpret_cast<value_type*>(&m_storage)}; // NOLINT
         }
      }
      auto begin() const noexcept -> const_iterator
      {
         if (m_data)
         {
            return const_iterator{m_data.get()};
         }
         else
         {
            return iterator{reinterpret_cast<const value_type*>(&m_storage)}; // NOLINT
         }
      }
      auto cbegin() const noexcept -> const_iterator
      {
         if (m_data)
         {
            return const_iterator{m_data.get()};
         }
         else
         {
            return iterator{reinterpret_cast<const value_type*>(&m_storage)}; // NOLINT
         }
      }

      auto end() noexcept -> iterator
      {
         if (m_data)
         {
            return iterator{m_data.get() + m_size};
         }
         else
         {
            return iterator{reinterpret_cast<value_type*>(&m_storage) + m_size}; // NOLINT
         }
      }
      auto end() const noexcept -> const_iterator
      {
         if (m_data)
         {
            return const_iterator{m_data.get() + m_size};
         }
         else
         {
            return iterator{reinterpret_cast<const value_type*>(&m_storage) + m_size}; // NOLINT
         }
      }
      auto cend() const noexcept -> const_iterator
      {
         if (m_data)
         {
            return const_iterator{m_data.get() + m_size};
         }
         else
         {
            return iterator{reinterpret_cast<const value_type*>(&m_storage) + m_size}; // NOLINT
         }
      }

      auto rbegin() noexcept -> reverse_iterator { return reverse_iterator{end()}; }
      auto rbegin() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cend()};
      }
      auto rcbegin() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cend()};
      }

      auto rend() noexcept -> reverse_iterator { return reverse_iterator{begin()}; }
      auto rend() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cbegin()};
      }
      auto rcend() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cbegin()};
      }

      [[nodiscard]] constexpr auto empty() const noexcept -> bool { return size() == 0; }
      [[nodiscard]] constexpr auto size() const noexcept -> size_type { return m_size; }
      [[nodiscard]] constexpr auto max_size() const noexcept -> size_type
      {
         return std::numeric_limits<difference_type>::max();
      }
      [[nodiscard]] constexpr auto capacity() const noexcept -> size_type { return m_capacity; }

      void clear() noexcept
      {
         destroy_range(begin(), end());
         m_size = 0;
      }

   private:
      void grow(size_type min_size = 0)
      {
         if (min_size > std::numeric_limits<difference_type>::max())
         {
            handle_bad_alloc_error("new size is too big");
         }

         const auto new_capacity =
            std::clamp(2 * m_capacity + 1, min_size, std::numeric_limits<size_type>::max());

         if (m_data) // already on the heap
         {
            try
            {
               auto new_alloc = std::make_unique<value_type[]>(new_capacity); // NOLINT

               if constexpr (std::movable<value_type>)
               {
                  std::uninitialized_move(begin(), end(), iterator{new_alloc.get()});
               }
               else
               {
                  std::uninitialized_copy(begin(), end(), iterator{new_alloc.get()});
               }

               m_data = std::move(new_alloc);
            }
            catch (const std::bad_alloc&)
            {
               handle_bad_alloc_error("Failed to allocate new memory");
            }
         }
         else // still using the static storage
         {
            try
            {
               m_data = std::make_unique<value_type[]>(new_capacity); // NOLINT

               if constexpr (std::movable<value_type>)
               {
                  std::uninitialized_move(begin(), end(), iterator{m_data.get()});
               }
               else
               {
                  std::uninitialized_copy(begin(), end(), iterator{m_data.get()});
               }
            }
            catch (const std::bad_alloc&)
            {
               handle_bad_alloc_error("Failed to allocate new memory");
            }
         }

         m_capacity = new_capacity;
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

   private:
      std::unique_ptr<value_type[]> m_data; // NOLINT
      size_type m_size{0};
      size_type m_capacity{0};
      details::static_array_storage<value_type, buffer_size_> m_storage;
   };

   /**
    * @class tiny_dynamic_array dynamic_array.hpp <epona_core/containers/dynamic_array.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, April 29th, 2020
    * @copyright MIT License.
    * @brief A dynamic array data structure that allows for a small static memory storage.
    *
    * @tparam any_, The type of objects that can be contained in the container.
    * @tparam buff_sz, The size of the static memory buffer in the container.
    * @tparam allocator_ The type of the allocator used by the container.
    */
   template <class any_, std::size_t buff_sz>
   class tiny_dynamic_array
   {
   public:
      using value_type = any_;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using iterator = random_access_iterator<value_type>;
      using const_iterator = random_access_iterator<const value_type>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
      explicit tiny_dynamic_array() : p_begin{get_first_element()}, cap{buff_sz} {}
      explicit tiny_dynamic_array(size_type count) requires std::default_initializable<value_type> :
         p_begin{get_first_element()},
         cap{buff_sz}
      {
         assign(count, value_type());
      }
      explicit tiny_dynamic_array(
         size_type count, const_reference value) requires std::copyable<value_type> :
         p_begin{get_first_element()},
         cap{buff_sz}
      {
         assign(count, value);
      }
      template <std::input_iterator it_>
      tiny_dynamic_array(it_ first, it_ last) requires std::copyable<value_type> :
         p_begin{get_first_element()},
         cap{buff_sz}
      {
         assign(first, last);
      }
      tiny_dynamic_array(const range_over<value_type> auto& r) :
         p_begin{get_first_element()}, cap{buff_sz}
      {
         assign(r);
      }
      tiny_dynamic_array(std::initializer_list<any_> init) requires std::copyable<value_type> :
         p_begin{get_first_element()},
         cap{buff_sz}
      {
         assign(init);
      }
      tiny_dynamic_array(const tiny_dynamic_array& other) :
         p_begin{get_first_element()}, cap{buff_sz}
      {
         if (!other.empty())
         {
            *this = other;
         }
      }
      tiny_dynamic_array(tiny_dynamic_array&& other) : p_begin{get_first_element()}, cap{buff_sz}
      {
         if (!other.empty())
         {
            *this = std::move(other);
         }
      }
      ~tiny_dynamic_array()
      {
         if (!is_static())
         {
            clear();

            if (p_begin)
            {
               free(static_cast<void*>(p_begin));
            }
         }
      }

      tiny_dynamic_array& operator=(const tiny_dynamic_array& rhs)
      {
         if (this == &rhs)
         {
            return *this;
         }

         size_type const other_sz = rhs.size();
         size_type curr_sz = size();

         if (curr_sz >= other_sz)
         {
            iterator new_end = begin();
            if (other_sz)
            {
               new_end = std::copy(rhs.begin(), rhs.end(), new_end);
            }

            destroy_range(new_end, end());

            elem_count = other_sz;

            return *this;
         }
         else
         {
            if (other_sz > capacity())
            {
               destroy_range(begin(), end());
               elem_count = curr_sz = 0;

               grow(other_sz);
            }
            else
            {
               std::copy(rhs.begin(), rhs.begin() + curr_sz, begin());
            }

            std::uninitialized_copy(rhs.begin() + curr_sz, rhs.end(), begin() + curr_sz);

            elem_count = other_sz;

            return *this;
         }
      }

      tiny_dynamic_array& operator=(tiny_dynamic_array&& rhs)
      {
         if (this == &rhs)
         {
            return *this;
         }

         // If not static, steal buffer.
         if (!rhs.is_static())
         {
            destroy_range(begin(), end());
            if (!is_static() && p_begin)
            {
               free(static_cast<void*>(p_begin));
            }

            elem_count = rhs.elem_count;
            rhs.elem_count = 0;

            cap = rhs.cap;
            rhs.cap = 0;

            p_begin = rhs.p_begin;
            rhs.p_begin = nullptr;

            return *this;
         }

         size_type const other_sz = rhs.size();
         size_type curr_sz = size();

         // if we have enough space, move the data from static buffer
         // and delete the leftover data we have.
         if (curr_sz >= other_sz)
         {
            iterator new_end = begin();
            if (other_sz)
            {
               new_end = std::move(rhs.begin(), rhs.end(), new_end);
            }

            destroy_range(new_end, end());

            elem_count = other_sz;

            rhs.clear();

            return *this;
         }
         else // resize and move data from static buffer.
         {
            if (other_sz > capacity())
            {
               destroy_range(begin(), end());
               elem_count = curr_sz = 0;

               grow(other_sz);
            }
            else if (curr_sz)
            {
               std::move(rhs.begin(), rhs.begin() + curr_sz, begin());
            }

            std::uninitialized_move(rhs.begin() + curr_sz, rhs.end(), begin() + curr_sz);

            elem_count = other_sz;

            rhs.clear();

            return *this;
         }
      }

      constexpr bool operator==(const tiny_dynamic_array<value_type, buff_sz>& rhs) const
         requires std::equality_comparable<value_type>
      {
         return std::equal(cbegin(), cend(), rhs.cbegin(), rhs.cend());
      }

      constexpr auto operator<=>(const tiny_dynamic_array<value_type, buff_sz>& rhs)
      {
         return std::lexicographical_compare_three_way(
            cbegin(), cend(), rhs.cbegin(), rhs.cend(), synth_three_way);
      }

      /**
       * @brief Replaces the content of the container with count copies of value value.
       *
       * @details Removes all elements currently present in the container and places count copies
       * of value value.All iterators, pointers and references to the elements of the container
       * are invalidated. The past-the-end iterator is also invalidated. To call this function,
       * #value_type must satisfy the <a
       * href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a>
       *
       * @param[in] count The new size of the container.
       * @param[in] value The value to initialize elements from the container with.
       */
      void assign(size_type count, const_reference value) requires std::copyable<value_type>
      {
         clear();

         if (count > capacity())
         {
            grow(count);
         }

         elem_count += count;

         std::uninitialized_fill(begin(), end(), value);
      }

      /**
       * @brief Replaces the contents of the container with copies of those in the range [first,
       * last).
       *
       * @details Removes all elements currently present in the container and places copies of
       * the range [first, last}. All iterators, pointers and references to the elements of the
       * container are invalidated. The past-the-end iterator is also invalidated. To call this
       * function, #value_type must satisfy the <a
       * href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a>
       *
       * @tparam it_ The type of the iterators. Must comply with the std::input_iterator
       * requirements.
       *
       * @param[in] first The first element to assign inclusive.
       * @param[in] last The last element to assign exclusive.
       */
      template <std::input_iterator it_>
      void assign(it_ first, it_ last) requires std::copyable<value_type>
      {
         clear();

         size_type const new_count = std::distance(first, last);
         if (new_count > capacity())
         {
            grow(new_count);
         }

         elem_count += new_count;

         std::uninitialized_copy(first, last, begin());
      }

      void assign(const range_over<value_type> auto& r) { assign(r.begin(), r.end()); }

      /**
       * @brief Replaces the contents of the container with the elements from the <a
       * href="https://en.cppreference.com/w/cpp/utility/initializer_list">std::initializer_list</a>
       * initializer_list.
       *
       * @details Removes all elements currently present in the container and places copies of
       * the elements contained within the <a
       * href="https://en.cppreference.com/w/cpp/utility/initializer_list">std::initializer_list</a>
       * initializer_list. All iterators, pointers and references to the elements of the
       * container are invalidated. The past-the-end iterator is also invalidated. To call this
       * function, #value_type must satisfy the <a
       * href="https://en.cppreference.com/w/cpp/concepts/copyable">std::copyable</a>
       *
       * @param[in] initializer_list The initializer list to copy the values from.
       */
      void assign(
         std::initializer_list<value_type> initializer_list) requires std::copyable<value_type>
      {
         assign(initializer_list.begin(), initializer_list.end());
      }

      /**
       * @brief Return a #reference to the element at the index position in the container.
       *
       * @details Return a #reference to the element at the index position in the container.
       * Performs bounds checking. Will not throw if ESL_NO_EXCEPTIONS is defined.
       *
       * @throw std::out_of_range if index >= size().
       *
       * @param[in] index The index position of the desired element.
       *
       * @return A #reference to the requested element.
       */
      reference at(size_type index)
      {
         if (index >= size())
         {
            handle_out_of_range_error("Index: " + std::to_string(index) + " is out of bounds");
         }
         else
         {
            return p_begin[index];
         }
      }
      /**
       * @brief Return a #const_reference to the element at the index position in the container.
       *
       * @details Return a #const_reference to the element at the index position in the
       * container. Performs bounds checking. Will not throw if ESL_NO_EXCEPTIONS is defined.
       *
       * @throw std::out_of_range if index >= size().
       *
       * @param[in] index The index position of the desired element.
       *
       * @return A #const_reference to the requested element.
       */
      const_reference at(size_type index) const
      {
         if (index >= size())
         {
            handle_out_of_range_error("Index: " + std::to_string(index) + " is out of bounds");
         }
         else
         {
            return p_begin[index];
         }
      }

      /**
       * @brief Return a #reference to the element at the index position in the container.
       *
       * @details Return a #reference to the element at the index position in the container. Does
       * not perform any bounds checking.
       *
       * @param[in] index The index position of the desired element.
       *
       * @return A #reference to the requested element.
       */
      reference operator[](size_type index) noexcept
      {
         assert(index < size() && "Index out of bounds.");

         return p_begin[index];
      }
      /**
       * @brief Return a #const_reference to the element at the index position in the container.
       *
       * @details Return a #const_reference to the element at the index position in the
       * container. Does not perform any bounds checking.
       *
       * @param[in] index The index position of the desired element.
       *
       * @return A #const_reference to the requested element.
       */
      const_reference operator[](size_type index) const noexcept
      {
         assert(index < size() && "Index out of bounds.");

         return p_begin[index];
      }

      /**
       * @brief Return a #reference to the first element in the container.
       *
       * @return A #reference to the first element in the container.
       */
      reference front() noexcept
      {
         assert(!empty() && "No elements in the container");
         return *begin();
      }
      /**
       * @brief Return a #const_reference to the first element in the container.
       *
       * @return A #const_reference to the first element in the container.
       */
      const_reference front() const noexcept
      {
         assert(!empty() && "No elements in the container");
         return *cbegin();
      }

      /**
       * @brief Return a #reference to the last element in the container.
       *
       * @return A #reference to the last element in the container.
       */
      reference back() noexcept
      {
         assert(!empty() && "No elements in the container");
         return *(end() - 1);
      }
      /**
       * @brief Return a #const_reference to the last element in the container.
       *
       * @return A #const_reference to the last element in the container.
       */
      const_reference back() const noexcept
      {
         assert(!empty() && "No elements in the container");
         return *(cend() - 1);
      }

      /**
       * @brief Return a #pointer to the container's memory.
       *
       * @return A #pointer to the container's memory
       */
      pointer data() noexcept { return pointer{&(*begin())}; }
      /**
       * @brief Return a #const_pointer to the container's memory.
       *
       * @return A #const_pointer to the container's memory
       */
      const_pointer data() const noexcept { return const_pointer{&(*cbegin())}; }

      iterator begin() noexcept { return iterator{p_begin}; }
      const_iterator begin() const noexcept { return const_iterator{p_begin}; }
      const_iterator cbegin() const noexcept { return const_iterator{p_begin}; }

      iterator end() noexcept { return iterator{begin() + size()}; }
      const_iterator end() const noexcept { return const_iterator{begin() + size()}; }
      const_iterator cend() const noexcept { return const_iterator{cbegin() + size()}; }

      reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
      const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{cend()}; }
      const_reverse_iterator rcbegin() const noexcept { return const_reverse_iterator{cend()}; }

      reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
      const_reverse_iterator rend() const noexcept { return const_reverse_iterator{cbegin()}; }
      const_reverse_iterator rcend() const noexcept { return const_reverse_iterator{cbegin()}; }

      /**
       * @brief Check if the container has no element.
       *
       * @return True if the container is empty, otherwise false.
       */
      [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
      /**
       * @brief Return the number of elements in the container.
       *
       * @return The size of the container.
       */
      size_type size() const noexcept { return elem_count; }
      /**
       * @brief Return the maximum size of the container.
       *
       * @return The maximum value held by the #difference_type.
       */
      size_type max_size() const noexcept { return std::numeric_limits<difference_type>::max(); }
      /**
       * @brief Return the container's capacity;
       *
       * @return The container's current memory capacity.
       */
      size_type capacity() const noexcept { return cap; }

      /**
       * @brief Increase the capacity of the container to a value greater than or equal to
       * new_cap.
       *
       * @details Increase the capacity of the dynamic_array to a value that's greater or equal
       * to new_cap. If new_cap is greater than the current capacity(), new storage is allocated,
       * otherwise the method does nothing. reserve() does not change the size of the
       * dynamic_array. If new_cap is greater than capacity(), all iterators, including the
       * past-the-end iterator, and all references to the elements are invalidated. Otherwise, no
       * iterators or references are invalidated.
       *
       * @param[in] new_cap The new capacity of the dynamic_array.
       */
      void reserve(size_type new_cap)
      {
         if (new_cap > capacity())
         {
            grow(new_cap);
         }
      }

      /**
       * @brief Erases all elements from the container
       *
       * @details Erases all elements from the container. After this call, size() return zero.
       * Invalidates any references, pointers, or iterators referring to contained elements. Any
       * past-the-end iterators are also invalidated. Leaves the capacity() of the dynamic_array
       * unchanged
       */
      void clear() noexcept
      {
         destroy_range(begin(), end());
         elem_count = 0;
      }

      iterator insert(const_iterator pos, const_reference value) requires std::copyable<value_type>
      {
         if (pos == cend())
         {
            push_back(value);

            return end() - 1;
         }

         assert(pos >= cbegin() && "Insertion iterator is out of bounds");
         assert(pos <= cend() && "Insertion iterator is past the end");

         iterator new_pos;
         if (size() >= capacity())
         {
            size_type offset = pos - cbegin();
            grow();
            new_pos = begin() + offset;
         }
         else
         {
            new_pos = begin() + (pos - cbegin());
         }

         new (static_cast<void*>(&(*end()))) value_type(std::move(back()));

         std::move_backward(new_pos, end() - 1, end());

         ++elem_count;

         const_pointer p_element = &value;
         if (pointer{&(*new_pos)} <= p_element && pointer{&(*end())} > p_element)
         {
            ++p_element;
         }

         *new_pos = *p_element;

         return new_pos;
      }

      iterator insert(const_iterator pos, value_type&& value) requires std::movable<value_type>
      {
         if (pos == cend())
         {
            push_back(std::move(value));

            return end() - 1;
         }

         assert(pos >= cbegin() && "Insertion iterator is out of bounds");
         assert(pos <= cend() && "Insertion iterator is past the end");

         iterator new_pos;
         if (size() >= capacity())
         {
            size_type offset = pos - cbegin();
            grow();
            new_pos = begin() + offset;
         }
         else
         {
            new_pos = begin() + (pos - cbegin());
         }

         new (static_cast<void*>(&(*end()))) value_type(std::move(back()));

         std::move_backward(new_pos, end() - 1, end());

         ++elem_count;

         pointer p_element = &value;
         if (pointer{&(*new_pos)} <= p_element && pointer{&(*end())} > p_element)
         {
            ++p_element;
         }

         *new_pos = std::move(*p_element);

         return new_pos;
      }

      iterator insert(const_iterator pos, size_type count,
         const_reference value) requires std::copyable<value_type>
      {
         size_type start_index = pos - cbegin();

         if (pos == cend())
         {
            if (size() + count >= capacity())
            {
               grow(size() + count);
            }

            std::uninitialized_fill_n(end(), count, value);

            elem_count += count;

            return begin() + start_index;
         }

         assert(pos >= cbegin() && "Insertion iterator is out of bounds");
         assert(pos <= cend() && "Insertion iterator is past the end");

         reserve(size() + count);

         iterator updated_pos = begin() + start_index;

         if (iterator old_end = end(); end() - updated_pos >= count)
         {
            std::uninitialized_move(end() - count, end(), end());

            elem_count += count;

            std::move_backward(updated_pos, old_end - count, old_end);
            std::fill_n(updated_pos, count, value);
         }
         else
         {
            size_type move_count = old_end - updated_pos;
            elem_count += count;

            std::uninitialized_move(updated_pos, old_end, end() - move_count);
            std::fill_n(updated_pos, move_count, value);
            std::uninitialized_fill_n(old_end, count - move_count, value);
         }

         return updated_pos;
      }

      template <std::input_iterator it_>
      iterator insert(const_iterator pos, it_ first, it_ last) requires std::copyable<value_type>
      {
         size_type start_index = pos - cbegin();
         difference_type count = std::distance(first, last);

         if (pos == cend())
         {
            if (size() + count >= capacity())
            {
               grow(size() + count);
            }

            std::uninitialized_copy(first, last, end());

            elem_count += count;

            return begin() + start_index;
         }

         assert(pos >= cbegin() && "Insertion iterator is out of bounds");
         assert(pos <= cend() && "Insertion iterator is past the end");

         reserve(size() + count);

         iterator updated_pos = begin() + start_index;
         if (iterator old_end = end(); end() - updated_pos >= count)
         {
            std::uninitialized_move(end() - count, end(), end());

            elem_count += count;

            std::move_backward(updated_pos, old_end - count, old_end);
            std::copy(first, last, updated_pos);
         }
         else
         {
            size_type move_count = old_end - updated_pos;
            elem_count += count;

            std::uninitialized_move(updated_pos, old_end, end() - move_count);

            for (auto it = updated_pos; count > 0; --count)
            {
               *it = *first;

               ++it;
               ++first;
            }

            std::uninitialized_copy(first, last, old_end);
         }

         return updated_pos;
      }

      iterator insert(const_iterator pos, const std::ranges::range auto& r)
      {
         return insert(pos, r.begin(), r.end());
      }

      iterator insert(const_iterator pos,
         std::initializer_list<value_type> init_list) requires std::copyable<value_type>
      {
         return insert(pos, init_list.begin(), init_list.end());
      }

      template <class... args_>
      iterator emplace(
         const_iterator pos, args_&&... args) requires std::constructible_from<value_type, args_...>
      {
         if (pos == cend())
         {
            emplace_back(std::forward<args_>(args)...);

            return end() - 1;
         }

         assert(pos >= cbegin() && "Insertion iterator is out of bounds");
         assert(pos <= cend() && "Insertion iterator is past the end");

         iterator new_pos;
         if (size() >= capacity())
         {
            size_type offset = pos - cbegin();
            grow();
            new_pos = begin() + offset;
         }
         else
         {
            new_pos = begin() + (pos - cbegin());
         }

         new (static_cast<void*>(&(*end()))) value_type(std::move(back()));

         std::move_backward(new_pos, end() - 1, end());

         ++elem_count;

         *new_pos = value_type(std::forward<args_>(args)...);

         return new_pos;
      }

      /**
       * @brief Erases the specified elements from the container.
       *
       * @details Erases the element at pos. Invalidates iterators and references at or after the
       * point of the erase, including the end() iterator. The iterator pos must be valid and
       * dereferenceable. Thus the end() iterator (which is valid, but is not dereferencable)
       * cannot be used as a value for pos.
       *
       * @param[in] pos The iterator to the element to remove.
       *
       * @return An iterator following the last removed element.
       */
      iterator erase(const_iterator pos)
      {
         if (pos == cend())
         {
            return end();
         }

         assert(pos >= cbegin() && "Insertion iterator is out of bounds");
         assert(pos <= cend() && "Insertion iterator is past the end");

         auto it = begin() + (pos - cbegin());

         std::move(it + 1, end(), it);
         pop_back();

         return it;
      }

      /**
       * @brief Erases the specified elements from the container.
       *
       * @details Removes the elements in the range [first, last). Invalidates iterators and
       * references at or after the point of the erase, including the end() iterator. The
       * iterator first does not need to be dereferenceable if first==last: erasing an empty
       * range is a no-op.
       *
       * @param[in] first The first element inclusive of the range.
       * @param[in] last The last element exclusive of the range.
       *
       * @return Iterator following the last removed element.
       */
      iterator erase(const_iterator first, const_iterator last)
      {
         if (first == last)
         {
            return begin() + (first - cbegin());
         }

         assert(first >= cbegin() && "first iterator is out of bounds");
         assert(last <= cend() && "last iterator is past the end");
         assert(first <= last && "first iterator is greater than last iterator");

         size_type const distance = std::distance(first, last);

         auto it_f = begin() + (first - cbegin());
         auto it_l = begin() + (last - cbegin());
         auto it = std::move(it_l, end(), it_f);

         destroy_range(it, end());

         elem_count -= distance;

         return it_f;
      }

      /**
       * @brief Appends the given element value to the end of the container.
       *
       * @details Initialize a new element as a copy of value at the end of the container. If the
       * new size() is greater than capacity() then all iterators and references (including the
       * past-the-end iterator) are invalidated. Otherwise only the past-the-end iterator is
       * invalidated. #value_type must meet the <a
       * href="https://en.cppreference.com/w/cpp/concepts/movable">std::movable</a> requirements
       * to use this function
       *
       * @param[in] value The value of the element to append.
       */
      void push_back(const_reference value) requires std::copyable<value_type>
      {
         if (size() >= capacity())
         {
            grow();
         }

         if constexpr (trivial<value_type>)
         {
            memcpy(static_cast<void*>(&(*end())), &value, sizeof(value_type));
         }
         else
         {
            new (static_cast<void*>(&(*end()))) value_type(value);
         }

         ++elem_count;
      };

      /**
       * @brief Appends the given element value to the end of the container.
       *
       * @details Move the value into the new element at the end of the container. If the new
       * size() is greater than capacity() then all iterators and references (including the
       * past-the-end iterator) are invalidated. Otherwise only the past-the-end iterator is
       * invalidated. #value_type must meet the <a
       * href="https://en.cppreference.com/w/cpp/concepts/movable">std::movable</a> requirements
       * to use this function
       *
       * @param[in] value The value of the element to append.
       */
      void push_back(value_type&& value) requires std::movable<value_type>
      {
         if (size() >= capacity())
         {
            grow();
         }

         new (static_cast<void*>(&(*end()))) value_type(std::move(value));

         ++elem_count;
      };

      /**
       * @brief Appends a new element at the end of the container.
       *
       * @details Constructs in place a new element at the end of the container. The arguments
       * args... are forwarded to the constructor of the #value_type. If the new size() is
       * greater than capacity() then all iterators and references (including the past-the-end
       * iterator) are invalidated. Otherwise only the past-the-end iterator is invalidated.
       * #value_type must meet the <a
       * href="https://en.cppreference.com/w/cpp/concepts/constructible_from">std::constructible_from<value_type,
       * args_...></a> requirements to use this function
       *
       * @tparam args_ The types of the arguments to construct the #value_type from.
       *
       * @param args The arguments to forward to the constructor of the #value_type.
       *
       * @return A #reference to the newly constructed element.
       */
      template <class... args_>
      reference emplace_back(args_&&... args) requires std::constructible_from<value_type, args_...>
      {
         if (size() >= capacity())
         {
            grow();
         }

         iterator last = end();
         new (static_cast<void*>(&(*last))) value_type(std::forward<args_>(args)...);

         ++elem_count;

         return *last;
      }

      /**
       * @brief Removes the last element of the container.
       *
       * @brief Removels the last element in the container unless empty. Iterators and references
       * to the last element, as well as the end() iterator, are invalidated.
       */
      void pop_back()
      {
         if (size() != 0)
         {
            --elem_count;
            end()->~value_type();
         }
      };

      /**
       * @brief Resizes the container to container count elements.
       *
       * @details  Resizes the container to container count elements. If the current size is
       * greater that count, the container is reduced to its first count elements. If the current
       * size is less than count, additional default-inserted elements are appended. #value_type
       * must meet the <a
       * href="https://en.cppreference.com/w/cpp/concepts/default_initializable">std::default_initializable</a>
       * requirements to use this function
       *
       * @param[in] count The new size of the container.
       */
      void resize(size_type count) requires std::default_initializable<value_type>
      {
         if (size() > count)
         {
            destroy_range(begin() + count, end());
            elem_count = count;
         }
         else if (size() < count)
         {
            if (capacity() < count)
            {
               grow(count);
            }

            for (size_type i = size(); i < count; ++i)
            {
               new (static_cast<void*>(p_begin + i)) value_type();
            }

            elem_count = count;
         }
      }

      /**
       * @brief Resizes the container to container count elements.
       *
       * @details  Resizes the container to container count elements. If the current size is
       * greater that count, the container is reduced to its first count elements. If the current
       * size is less than count, additional copies of value are appended. #value_type must meet
       * the <a
       * href="https://en.cppreference.com/w/cpp/concepts/default_initializable">std::default_initializable</a>
       * requirements to use this function
       *
       * @param[in] count The new size of the container.
       * @param[in] value The value to initialize the new elements with.
       */
      void resize(size_type count, const_reference value) requires std::copyable<value_type>
      {
         if (size() > count)
         {
            destroy_range(begin() + count, end());
            elem_count = count;
         }
         else if (size() < count)
         {
            if (capacity() < count)
            {
               grow(count);
            }

            std::uninitialized_fill(end(), begin() + count, value);

            elem_count = count;
         }
      }

   private:
      /**
       * @brief Check if the container is currently using the static memory buffer.
       *
       * @return True if the container is using the static memory buffer, otherwise false.
       */
      bool is_static() const noexcept { return p_begin == get_first_element(); }

      pointer get_first_element() const
      {
         return const_cast<pointer>(reinterpret_cast<const_pointer>(&storage));
      }

      void grow(size_type min_size = 0)
      {
         if (min_size > std::numeric_limits<difference_type>::max())
         {
            handle_bad_alloc_error("Hybrid dynamic_array capacity overflow during allocation.");
         }

         if constexpr (trivial<value_type>)
         {
            size_type const new_capacity =
               std::clamp(2 * capacity() + 1, min_size, std::numeric_limits<size_type>::max());

            if (p_begin == get_first_element())
            {
               void* p_new = std::malloc(new_capacity * sizeof(value_type));
               if (!p_new)
               {
                  handle_bad_alloc_error(
                     "Hybrid dynamic_array capacity overflow during reallocation.");
               }

               memcpy(p_new, p_begin, size() * sizeof(value_type));

               p_begin = static_cast<pointer>(p_new);
            }
            else
            {
               void* p_new =
                  std::realloc(static_cast<void*>(p_begin), new_capacity * sizeof(value_type));
               if (!p_new)
               {
                  handle_bad_alloc_error(
                     "Hybrid dynamic_array capacity overflow during reallocation.");
               }

               p_begin = static_cast<pointer>(p_new);
            }

            cap = new_capacity;
         }
         else
         {
            size_type new_cap =
               std::clamp(2 * capacity() + 1, min_size, std::numeric_limits<size_type>::max());

            pointer p_new{nullptr};
            if (is_static()) // memory is currently static
            {
               p_new = static_cast<value_type*>(std::malloc(new_cap * sizeof(value_type)));
               if (!p_new)
               {
                  handle_bad_alloc_error("hybrid dynamic_array allocation error");
               }

               if constexpr (std::movable<value_type>)
               {
                  std::uninitialized_move(begin(), end(), iterator{p_new});
               }
               else
               {
                  std::uninitialized_copy(begin(), end(), iterator{p_new});
               }

               destroy_range(begin(), end());
            }
            else
            {
               p_new = static_cast<value_type*>(
                  std::realloc(static_cast<void*>(p_begin), new_cap * sizeof(value_type)));
               if (!p_new)
               {
                  handle_bad_alloc_error("Hybrid dynamic_array allocation error");
               }
            }

            p_begin = p_new;
            cap = new_cap;
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

   private:
      pointer p_begin{nullptr};
      size_type elem_count{0};
      size_type cap{0};
      details::static_array_storage<value_type, buff_sz> storage;
   };

   template <class any_>
   using dynamic_array = tiny_dynamic_array<any_, 0>;
} // namespace util
