/**
 * @file dynamic_array.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, April 23rd, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "util/compare.hpp"
#include "util/concepts.hpp"
#include "util/containers/detail/error_handling.hpp"
#include "util/iterators/input_iterator.hpp"
#include "util/iterators/random_access_iterator.hpp"

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <memory_resource>
#include <type_traits>

namespace util
{
   template <class any_, std::size_t buffer_size_, allocator allocator_ = std::allocator<any_>>
   class small_dynamic_array
   {
      using allocator_traits = std::allocator_traits<allocator_>;

      static inline constexpr bool is_nothrow_swappable =
         std::allocator_traits<allocator_>::propagate_on_container_swap::value ||
         std::allocator_traits<allocator_>::is_always_equal::value;

   public:
      using value_type = any_;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
      using allocator_type = allocator_;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
      using iterator = random_access_iterator<value_type>;
      using const_iterator = random_access_iterator<const value_type>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      explicit constexpr small_dynamic_array() noexcept(noexcept(allocator_type())) = default;
      explicit constexpr small_dynamic_array(const allocator_type& alloc) noexcept :
         m_allocator{alloc}
      {}
      explicit constexpr small_dynamic_array(
         size_type count,
         const allocator_type& alloc =
            allocator_type{}) requires std::default_initializable<value_type> : m_allocator{alloc}
      {
         assign(count, value_type{});
      }
      explicit constexpr small_dynamic_array(
         size_type count, const_reference value,
         const allocator_type& alloc = allocator_type{}) requires std::copyable<value_type> :
         m_allocator{alloc}
      {
         assign(count, value);
      }
      template <std::input_iterator it_>
      constexpr small_dynamic_array(
         it_ first, it_ last,
         const allocator_type& alloc = allocator_type{}) requires std::copyable<value_type> :
         m_allocator{alloc}
      {
         assign(first, last);
      }
      constexpr small_dynamic_array(
         std::initializer_list<any_> init,
         const allocator_type& alloc = allocator_type{}) requires std::copyable<value_type> :
         m_allocator{alloc}
      {
         assign(init);
      }
      constexpr small_dynamic_array(const small_dynamic_array& other)
      {
         if (!other.empty())
         {
            *this = other;
         }
      }
      constexpr small_dynamic_array(const small_dynamic_array& other, const allocator_& alloc)
      {
         clear();
         if (!is_static())
         {
            allocator_traits::deallocate(m_allocator, m_pbegin, capacity());
         }
         reset_to_static();

         m_allocator = alloc;

         assign(other.begin(), other.end());
      }
      constexpr small_dynamic_array(small_dynamic_array&& other) noexcept
      {
         if (!other.empty())
         {
            *this = std::move(other);
         }
      }
      constexpr small_dynamic_array(small_dynamic_array&& other, const allocator_type& alloc)
      {
         clear();
         if (!is_static())
         {
            allocator_traits::deallocate(m_allocator, m_pbegin, capacity());
         }
         reset_to_static();

         m_allocator = alloc;

         using mi = std::move_iterator<iterator>;
         assign(mi{other.begin()}, mi{other.end()});

         other.reset_to_static();
      }
      constexpr ~small_dynamic_array()
      {
         clear();

         if (!is_static() && m_pbegin)
         {
            allocator_traits::deallocate(m_allocator, m_pbegin, capacity());
         }

         reset_to_static();
      }

      constexpr auto operator=(const small_dynamic_array& rhs) -> small_dynamic_array&
      {
         if (this != &rhs)
         {
            copy_assign_alloc(rhs);
            assign(rhs.begin(), rhs.end());
         }

         return *this;
      }

      constexpr auto operator=(small_dynamic_array&& rhs) noexcept(
         allocator_type::propagate_on_container_move_assignment::value ||
         allocator_type::is_always_equal::value) -> small_dynamic_array&
      {
         move_assign(rhs,
                     std::integral_constant<
                        bool, allocator_traits::propagate_on_container_move_assignment::value>());

         return *this;
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
      void assign(it_ first, it_ last)
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

      constexpr auto get_allocator() const noexcept -> allocator_type { return m_allocator; }

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
         destroy(begin(), end());
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

         construct(offset(size()), std::move(back()));

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

         construct(offset(size()), std::move(back()));

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

         destroy(it, end());

         m_size -= distance;

         return it_f;
      }

      constexpr void push_back(const_reference value) requires std::copyable<value_type>
      {
         emplace_back(value);
      };

      constexpr void push_back(value_type&& value) requires std::movable<value_type>
      {
         emplace_back(std::move(value));
      };

      constexpr auto emplace_back(auto&&... args) -> reference
         requires std::constructible_from<value_type, decltype(args)...>
      {
         if (size() >= capacity())
         {
            grow();
         }

         construct(m_pbegin + size(), std::forward<decltype(args)>(args)...);
         ++m_size;

         return back();
      }

      constexpr void pop_back()
      {
         if (size() != 0)
         {
            destroy(offset(size()));
            --m_size;
         }
      };

      constexpr void resize(size_type count) requires std::default_initializable<value_type>
      {
         if (size() > count)
         {
            destroy(begin() + count, end());
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
               construct(offset(i), value_type{});
            }

            m_size = count;
         }
      }

      constexpr void resize(size_type count,
                            const_reference value) requires std::copyable<value_type>
      {
         if (size() > count)
         {
            destroy(begin() + count, end());
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
         return const_cast<pointer>(reinterpret_cast<const_pointer>(&m_static_storage)); // NOLINT
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

         const auto new_capacity = compute_new_capacity(std::max(m_capacity + 1, min_size));

         try
         {
            auto new_elements = allocator_traits::allocate(m_allocator, new_capacity);

            if constexpr (std::movable<value_type>)
            {
               std::uninitialized_move(begin(), end(), iterator{new_elements});
            }
            else
            {
               std::uninitialized_copy(begin(), end(), iterator{new_elements});
            }

            destroy(begin(), end());

            if (!is_static())
            {
               allocator_traits::deallocate(m_allocator, m_pbegin, capacity());
            }

            m_pbegin = new_elements;
            m_capacity = new_capacity;
         }
         catch (const std::bad_alloc&)
         {
            handle_bad_alloc_error("Failed to allocate new memory");
         }
      }

      constexpr void copy_assign_alloc(const small_dynamic_array& other)
      {
         if constexpr (allocator_traits::propagate_on_container_copy_assignment::value)
         {
            if (m_allocator != other.m_allocator)
            {
               clear();

               if (!is_static())
               {
                  allocator_traits::deallocate(m_allocator, m_pbegin, capacity());
               }

               reset_to_static();
            }

            m_allocator = get_allocator();
         }
      }

      constexpr void move_assign_alloc(const small_dynamic_array& other)
      {
         if constexpr (allocator_traits::propagate_on_container_move_assignment::value)
         {
            m_allocator = std::move(other.m_allocator);
         }
      }

      constexpr void move_assign(small_dynamic_array& other, [[maybe_unused]] std::false_type u)
      {
         if (m_allocator != other.m_allocator)
         {
            using mi = std::move_iterator<iterator>;
            assign(mi{other.begin()}, mi{other.end()});

            other.reset_to_static();
         }
         else
         {
            move_assign(other, std::true_type{});
         }
      }
      constexpr void
      move_assign(small_dynamic_array& other,
                  [[maybe_unused]] std::true_type u) requires std::movable<value_type>
      {
         move_assign_alloc(other);

         if (!other.is_static())
         {
            if (!is_static())
            {
               clear();

               allocator_traits::deallocate(m_allocator, m_pbegin, capacity());

               reset_to_static();
            }

            m_size = other.size();
            m_capacity = other.capacity();
            m_pbegin = other.m_pbegin;
         }
         else
         {
            using mi = std::move_iterator<iterator>;
            assign(mi{other.begin()}, mi{other.end()});
         }

         other.reset_to_static();
      }

      constexpr void reset_to_static()
      {
         m_pbegin = get_first_element();
         m_size = 0;
         m_capacity = buffer_size_;
      }

      constexpr void construct(pointer plocation, auto&&... args)
      {
         if (is_static())
         {
            std::construct_at(plocation, std::forward<decltype(args)>(args)...);
         }
         else
         {
            allocator_traits::construct(m_allocator, plocation,
                                        std::forward<decltype(args)>(args)...);
         }
      }
      constexpr void destroy(pointer plocation)
      {
         if (is_static())
         {
            std::destroy_at(plocation);
         }
         else
         {
            allocator_traits::destroy(m_allocator, plocation);
         }
      }
      constexpr void destroy(iterator beg, iterator end)
      {
         if (is_static())
         {
            std::destroy(beg, end);
         }
         else
         {
            std::for_each(beg, end, [&](auto& value) {
               allocator_traits::destroy(m_allocator, std::addressof(value));
            });
         }
      }

      static constexpr auto compute_new_capacity(std::size_t min_capacity) -> std::size_t
      {
         constexpr auto max_capacity = std::size_t{1}
            << (std::numeric_limits<std::size_t>::digits - 1);

         if (min_capacity > max_capacity)
         {
            return max_capacity;
         }

         --min_capacity;

         for (auto i = 1U; i < std::numeric_limits<std::size_t>::digits; i *= 2)
         {
            min_capacity |= min_capacity >> i;
         }

         return ++min_capacity;
      }

      constexpr auto offset(size_type i) noexcept -> pointer { return m_pbegin + i; }
      constexpr auto offset(size_type i) const noexcept -> const_pointer { return m_pbegin + i; }

      pointer m_pbegin{get_first_element()};
      alignas(alignof(any_)) std::array<std::byte, sizeof(any_) * buffer_size_> m_static_storage;
      size_type m_size{0};
      size_type m_capacity{buffer_size_};
      allocator_type m_allocator{};
   }; // namespace util

   template <class type_, std::size_t first_size_, std::size_t second_size_, allocator allocator_>
   constexpr auto operator<=>(const small_dynamic_array<type_, first_size_, allocator_>& lhs,
                              const small_dynamic_array<type_, second_size_, allocator_>& rhs)
   {
      return std::lexicographical_compare_three_way(lhs.cbegin(), lhs.cend(), rhs.cbegin(),
                                                    rhs.cend(), synth_three_way);
   }

   template <std::equality_comparable type_, std::size_t first_size_, std::size_t second_size_,
             allocator allocator_>
   constexpr auto operator==(const small_dynamic_array<type_, first_size_, allocator_>& lhs,
                             const small_dynamic_array<type_, second_size_, allocator_>& rhs)
      -> bool
   {
      return std::equal(lhs.begin(), lhs.end(), rhs.cbegin(), rhs.cend());
   }

   template <class any_, allocator allocator_ = std::allocator<any_>>
   using dynamic_array = small_dynamic_array<any_, 0, allocator_>;

   namespace pmr
   {
      template <class any_, size_t size_>
      using small_dynamic_array =
         small_dynamic_array<any_, size_, std::pmr::polymorphic_allocator<any_>>;

      template <class any_>
      using dynamic_array = small_dynamic_array<any_, 0>;
   } // namespace pmr
} // namespace util

namespace std // NOLINT
{
   template <class type_, size_t size_, util::allocator allocator_>
   constexpr auto erase(util::small_dynamic_array<type_, size_, allocator_>& c, const auto& value)
      -> typename decltype(c)::size_type
   {
      auto it = std::remove(c.begin(), c.end(), value);
      auto r = std::distance(it, c.end());
      c.erase(it, c.end());
      return r;
   }

   template <class type_, size_t size_, util::allocator allocator_>
   constexpr auto erase_if(util::small_dynamic_array<type_, size_, allocator_>& c, auto pred) ->
      typename decltype(c)::size_type
   {
      auto it = std::remove(c.begin(), c.end(), pred);
      auto r = std::distance(it, c.end());
      c.erase(it, c.end());
      return r;
   }
} // namespace std
