/**
 * @file dynamic_array.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, April 23rd, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <util/compare.hpp>
#include <util/concepts.hpp>
#include <util/containers/detail/growth_policy.hpp>
#include <util/containers/error_handling.hpp>
#include <util/iterators/input_iterator.hpp>
#include <util/iterators/random_access_iterator.hpp>
#include <util/monad/either.hpp>

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
   namespace detail
   {
      /**
       * @struct static_array_storage details.hpp <ESL/containers/small_dynamic_array.hpp>
       * @author wmbat wmbat@protonmail.com
       * @date Monday, April 29th, 2020
       * @copyright MIT License.
       * @brief A small buffer class that represents an array.
       *
       * @tparam any_, The type of objects that can be contained in the static storage of the
       * container.
       * @tparam buff_sz, The size of the storage.
       */
      template <class any_, std::size_t buff_sz>
      struct static_array_storage
      {
         std::array<std::aligned_storage_t<sizeof(any_), alignof(any_)>, buff_sz> buffer;
      };

      template <class any_>
      struct alignas(alignof(any_)) static_array_storage<any_, 0>
      {
      };
   } // namespace detail

   template <class any_, std::size_t buffer_size_,
      class policy_ = detail::growth_policy::power_of_two>
   class small_dynamic_array
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
      explicit constexpr small_dynamic_array() = default;
      explicit constexpr small_dynamic_array(
         size_type count) requires std::default_initializable<value_type>
      {
         assign(count, value_type{});
      }
      explicit small_dynamic_array(
         size_type count, const_reference value) requires std::copyable<value_type>
      {
         assign(count, value);
      }
      template <std::input_iterator it_>
      small_dynamic_array(it_ first, it_ last) requires std::copyable<value_type>
      {
         assign(first, last);
      }
      small_dynamic_array(std::initializer_list<any_> init) requires std::copyable<value_type>
      {
         assign(init);
      }
      small_dynamic_array(const small_dynamic_array& other)
      {
         if (!other.empty())
         {
            *this = other;
         }
      }
      small_dynamic_array(small_dynamic_array&& other) noexcept
      {
         if (!other.empty())
         {
            *this = std::move(other);
         }
      }
      constexpr ~small_dynamic_array()
      {
         if (!is_static() && m_pbegin)
         {
            delete[] m_pbegin;
            m_pbegin = nullptr;
         }
      }

      constexpr auto operator=(const small_dynamic_array& rhs) -> small_dynamic_array&
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
         }
         else
         {
            if (other_sz > capacity())
            {
               destroy_range(begin(), end());
               m_size = curr_sz = 0;

               grow(other_sz);
            }
            else
            {
               std::copy(rhs.begin(), rhs.begin() + curr_sz, begin());
            }

            if constexpr (trivial<value_type>)
            {
               std::uninitialized_copy(rhs.begin() + curr_sz, rhs.end(), begin() + curr_sz);
            }
            else
            {
               std::copy(rhs.begin() + curr_sz, rhs.end(), begin() + curr_sz);
            }
         }

         m_size = other_sz;

         return *this;
      }

      constexpr auto operator=(small_dynamic_array&& rhs) noexcept -> small_dynamic_array&
      {
         if (this == &rhs)
         {
            return *this;
         }

         // If not static, steal buffer.
         if (!rhs.is_static())
         {
            if (is_static())
            {
               destroy_range(begin(), end());
            }
            else
            {
               delete[] m_pbegin;
            }

            m_size = rhs.size();
            m_capacity = rhs.capacity();
            m_pbegin = rhs.m_pbegin;

            rhs.reset_to_static();

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

            m_size = other_sz;

            rhs.clear();

            return *this;
         }
         else // resize and move data from static buffer.
         {
            if (other_sz > capacity())
            {
               destroy_range(begin(), end());
               m_size = curr_sz = 0;

               grow(other_sz);
            }
            else if (curr_sz)
            {
               std::move(rhs.begin(), rhs.begin() + curr_sz, begin());
            }

            std::uninitialized_move(rhs.begin() + curr_sz, rhs.end(), begin() + curr_sz);

            m_size = other_sz;

            rhs.clear();

            return *this;
         }
      }

      constexpr auto operator==(const small_dynamic_array& rhs) const
         -> bool requires std::equality_comparable<value_type>
      {
         return std::equal(cbegin(), cend(), rhs.cbegin(), rhs.cend());
      }

      constexpr auto operator<=>(const small_dynamic_array& rhs)
      {
         return std::lexicographical_compare_three_way(
            cbegin(), cend(), rhs.cbegin(), rhs.cend(), synth_three_way);
      }

      constexpr void assign(size_type count, const_reference value) noexcept
         requires std::copyable<value_type>
      {
         clear();

         if (count > m_capacity)
         {
            grow(count);
         }

         m_size = count;

         std::uninitialized_fill(begin(), end(), value);
      }

      template <std::input_iterator it_>
      void assign(it_ first, it_ last) requires std::copyable<value_type>
      {
         clear();

         const size_type new_count = std::distance(first, last);
         if (new_count > capacity())
         {
            grow(new_count);
         }

         m_size = new_count;

         std::uninitialized_copy(first, last, begin());
      }

      // clang-format off
      void assign(std::initializer_list<value_type> initializer_list) 
         requires std::copyable<value_type>
      // clang-format on
      {
         assign(initializer_list.begin(), initializer_list.end());
      }

      constexpr auto at(size_type pos) -> reference
      {
         if (index >= size())
         {
            handle_out_of_range_error("Index: " + std::to_string(pos) + " is out of bounds");
         }
         else
         {
            return m_pbegin[pos];
         }
      }
      constexpr auto at(size_type pos) const -> const_reference
      {
         if (index >= size())
         {
            handle_out_of_range_error("Index: " + std::to_string(pos) + " is out of bounds");
         }
         else
         {
            return m_pbegin[pos];
         }
      }

      constexpr auto operator[](size_type index) noexcept -> reference
      {
         assert(index < size() && "Index out of bounds.");

         return m_pbegin[index];
      }
      constexpr auto operator[](size_type index) const noexcept -> const_reference
      {
         assert(index < size() && "Index out of bounds.");

         return m_pbegin[index];
      }

      constexpr auto front() noexcept -> reference
      {
         assert(!empty() && "No elements in the container");
         return *begin();
      }
      constexpr auto front() const noexcept -> const_reference
      {
         assert(!empty() && "No elements in the container");
         return *cbegin();
      }

      constexpr auto back() noexcept -> reference
      {
         assert(!empty() && "No elements in the container");
         return *(end() - 1);
      }
      constexpr auto back() const noexcept -> const_reference
      {
         assert(!empty() && "No elements in the container");
         return *(cend() - 1);
      }

      constexpr auto data() noexcept -> pointer { return pointer{&(*begin())}; }
      constexpr auto data() const noexcept -> const_pointer { return const_pointer{&(*cbegin())}; }

      constexpr auto begin() noexcept -> iterator { return iterator{m_pbegin}; }
      constexpr auto begin() const noexcept -> const_iterator { return const_iterator{m_pbegin}; }
      constexpr auto cbegin() const noexcept -> const_iterator { return const_iterator{m_pbegin}; }

      constexpr auto end() noexcept -> iterator { return iterator{m_pbegin + m_size}; }
      constexpr auto end() const noexcept -> const_iterator
      {
         return const_iterator{m_pbegin + m_size};
      }
      constexpr auto cend() const noexcept -> const_iterator
      {
         return const_iterator{m_pbegin + m_size};
      }

      constexpr auto rbegin() noexcept -> reverse_iterator { return reverse_iterator{end()}; }
      constexpr auto rbegin() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cend()};
      }
      constexpr auto rcbegin() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cend()};
      }

      constexpr auto rend() noexcept -> reverse_iterator { return reverse_iterator{begin()}; }
      constexpr auto rend() const noexcept -> const_reverse_iterator
      {
         return const_reverse_iterator{cbegin()};
      }
      constexpr auto rcend() const noexcept -> const_reverse_iterator
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
      constexpr void reserve(size_type new_cap)
      {
         if (new_cap > capacity())
         {
            grow(new_cap);
         }
      }

      constexpr void clear() noexcept
      {
         destroy_range(begin(), end());
         m_size = 0;
      }

      constexpr auto insert(const_iterator pos, const_reference value) -> iterator
         requires std::copyable<value_type>
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

         new (&(*end())) value_type(std::move(back()));

         std::move_backward(new_pos, end() - 1, end());

         ++m_size;

         const_pointer p_element = &value;
         if (pointer{&(*new_pos)} <= p_element && pointer{&(*end())} > p_element)
         {
            ++p_element;
         }

         *new_pos = *p_element;

         return new_pos;
      }

      constexpr auto insert(const_iterator pos, value_type&& value) -> iterator
         requires std::movable<value_type>
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

         new (&(*end())) value_type(std::move(back()));

         std::move_backward(new_pos, end() - 1, end());

         ++m_size;

         pointer p_element = &value;
         if (pointer{&(*new_pos)} <= p_element && pointer{&(*end())} > p_element)
         {
            ++p_element;
         }

         *new_pos = std::move(*p_element);

         return new_pos;
      }

      constexpr auto insert(const_iterator pos, size_type count, const_reference value) -> iterator
         requires std::copyable<value_type>
      {
         size_type start_index = pos - cbegin();

         if (pos == cend())
         {
            if (size() + count >= capacity())
            {
               grow(size() + count);
            }

            std::uninitialized_fill_n(end(), count, value);

            m_size += count;

            return begin() + start_index;
         }

         assert(pos >= cbegin() && "Insertion iterator is out of bounds");
         assert(pos <= cend() && "Insertion iterator is past the end");

         reserve(size() + count);

         iterator updated_pos = begin() + start_index;

         if (iterator old_end = end(); end() - updated_pos >= count)
         {
            std::uninitialized_move(end() - count, end(), end());

            m_size += count;

            std::move_backward(updated_pos, old_end - count, old_end);
            std::fill_n(updated_pos, count, value);
         }
         else
         {
            size_type move_count = old_end - updated_pos;
            m_size += count;

            std::uninitialized_move(updated_pos, old_end, end() - move_count);
            std::fill_n(updated_pos, move_count, value);
            std::uninitialized_fill_n(old_end, count - move_count, value);
         }

         return updated_pos;
      }

      template <std::input_iterator it_>
      constexpr auto insert(const_iterator pos, it_ first, it_ last) -> iterator
         requires std::copyable<value_type>
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

            m_size += count;

            return begin() + start_index;
         }

         assert(pos >= cbegin() && "Insertion iterator is out of bounds");
         assert(pos <= cend() && "Insertion iterator is past the end");

         reserve(size() + count);

         iterator updated_pos = begin() + start_index;
         if (iterator old_end = end(); end() - updated_pos >= count)
         {
            std::uninitialized_move(end() - count, end(), end());

            m_size += count;

            std::move_backward(updated_pos, old_end - count, old_end);
            std::copy(first, last, updated_pos);
         }
         else
         {
            size_type move_count = old_end - updated_pos;
            m_size += count;

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

      constexpr auto insert(const_iterator pos, const std::ranges::range auto& r) -> iterator
      {
         return insert(pos, r.begin(), r.end());
      }

      constexpr auto insert(const_iterator pos, std::initializer_list<value_type> init_list)
         -> iterator requires std::copyable<value_type>
      {
         return insert(pos, init_list.begin(), init_list.end());
      }

      template <class... args_>
      constexpr auto emplace(const_iterator pos, args_&&... args) -> iterator
         requires std::constructible_from<value_type, args_...>
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

         new (&(*end())) value_type(std::move(back()));

         std::move_backward(new_pos, end() - 1, end());

         ++m_size;

         *new_pos = value_type(std::forward<args_>(args)...);

         return new_pos;
      }

      constexpr auto erase(const_iterator pos) -> iterator
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

      constexpr auto erase(const_iterator first, const_iterator last) -> iterator
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

         m_size -= distance;

         return it_f;
      }

      constexpr void push_back(const_reference value) requires std::copyable<value_type>
      {
         if (size() >= capacity())
         {
            grow();
         }

         new (&(*end())) value_type(value);

         ++m_size;
      };

      constexpr void push_back(value_type&& value) requires std::movable<value_type>
      {
         if (size() >= capacity())
         {
            grow();
         }

         new (&(*end())) value_type(std::move(value));

         ++m_size;
      };

      template <class... args_>
      constexpr auto emplace_back(args_&&... args) -> reference
         requires std::constructible_from<value_type, args_...>
      {
         if (size() >= capacity())
         {
            grow();
         }

         iterator last = end();
         new (&(*last)) value_type(std::forward<args_>(args)...);

         ++m_size;

         return *last;
      }

      constexpr void pop_back()
      {
         if (size() != 0)
         {
            --m_size;
            end()->~value_type();
         }
      };

      constexpr void resize(size_type count) requires std::default_initializable<value_type>
      {
         if (size() > count)
         {
            destroy_range(begin() + count, end());
            m_size = count;
         }
         else if (size() < count)
         {
            if (capacity() < count)
            {
               grow(count);
            }

            for (size_type i = size(); i < count; ++i)
            {
               new (m_pbegin + i) value_type();
            }

            m_size = count;
         }
      }

      constexpr void resize(
         size_type count, const_reference value) requires std::copyable<value_type>
      {
         if (size() > count)
         {
            destroy_range(begin() + count, end());
            m_size = count;
         }
         else if (size() < count)
         {
            if (capacity() < count)
            {
               grow(count);
            }

            std::uninitialized_fill(end(), begin() + count, value);

            m_size = count;
         }
      }

   private:
      [[nodiscard]] constexpr auto is_static() const noexcept -> bool
      {
         return m_pbegin == get_first_element();
      }

      constexpr auto get_first_element() const -> pointer
      {
         return const_cast<pointer>(reinterpret_cast<const_pointer>(&m_storage)); // NOLINT
      }

      constexpr void grow(size_type min_size = 0)
      {
         if (min_size > std::numeric_limits<difference_type>::max())
         {
            handle_bad_alloc_error("small_dynamic_array capacity overflow during allocation");
         }

         if (capacity() == std::numeric_limits<difference_type>::max())
         {
            handle_bad_alloc_error("small_dynamic_array capacity unable to grow");
         }

         const auto new_capacity =
            policy_::compute_new_capacity(std::max(m_capacity + 1, min_size));

         /*
         const auto new_capacity =
            std::clamp(2 * m_capacity + 1, min_size, std::numeric_limits<size_type>::max());
         */

         try
         {
            auto new_elements = new value_type[new_capacity]; // NOLINT

            if constexpr (std::movable<value_type>)
            {
               if constexpr (trivial<value_type>)
               {
                  std::uninitialized_move(begin(), end(), iterator{new_elements});
               }
               else
               {
                  std::move(begin(), end(), iterator{new_elements});
               }
            }
            else
            {
               if constexpr (trivial<value_type>)
               {
                  std::uninitialized_copy(begin(), end(), iterator{new_elements});
               }
               else
               {
                  std::copy(begin(), end(), iterator{new_elements});
               }
            }

            destroy_range(begin(), end());

            if (!is_static())
            {
               delete[] m_pbegin;
            }

            m_pbegin = new_elements;
            m_capacity = new_capacity;
         }
         catch (const std::bad_alloc&)
         {
            handle_bad_alloc_error("Failed to allocate new memory");
         }

         m_capacity = new_capacity;
      }

      static constexpr void destroy_range(iterator first, iterator last)
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

      void reset_to_static()
      {
         m_pbegin = get_first_element();
         m_size = 0;
         m_capacity = buffer_size_;
      }

   private:
      pointer m_pbegin{get_first_element()};
      size_type m_size{0};
      size_type m_capacity{buffer_size_};
      detail::static_array_storage<value_type, buffer_size_> m_storage;
   }; // namespace util

   template <class any_, class policy_ = detail::growth_policy::power_of_two>
   using dynamic_array = small_dynamic_array<any_, 0, policy_>;
} // namespace util
