/**
 * @file flat_map.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 6th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/containers/details.hpp"
#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/details/concepts.hpp"
#include "epona_core/details/error_handling.hpp"
#include "epona_core/memory/details.hpp"
#include "epona_core/memory/multipool_allocator.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <tuple>
#include <utility>

namespace core
{
   template <std::equality_comparable key_, class val_, std::size_t buff_sz,
      complex_allocator<std::pair<key_, val_>> allocator_, class compare_ = std::less<key_>>
   class tiny_flat_map
   {
      using container_type = tiny_dynamic_array<std::pair<key_, val_>, buff_sz, allocator_>;

   public:
      using key_type = key_;
      using mapped_type = val_;
      using value_type = std::pair<key_, val_>;
      using size_type = typename container_type::size_type;
      using difference_type = typename container_type::difference_type;
      using key_compare = compare_;
      using allocator_type = allocator_;
      using reference = typename container_type::reference;
      using const_reference = typename container_type::const_reference;
      using pointer = typename container_type::pointer;
      using const_pointer = typename container_type::const_pointer;
      using iterator = typename container_type::iterator;
      using const_iterator = typename container_type::const_iterator;
      using reverse_iterator = typename container_type::reverse_iterator;
      using const_reverse_iterator = typename container_type::const_reverse_iterator;

   public:
      tiny_flat_map(allocator_type* p_allocator) : data(p_allocator) {}

      mapped_type& at(const key_type& key)
      {
         auto it = find(key);
         if (it == end())
         {
            handle_out_of_range_error("key is not present in container");
         }

         return it->second;
      }
      const mapped_type& at(const key_type& key) const
      {
         auto it = find(key);
         if (it == cend())
         {
            handle_out_of_range_error("key is not present in container");
         }

         return it->second;
      }

      iterator begin() noexcept { return data.begin(); }
      const_iterator begin() const noexcept { return data.begin(); }
      const_iterator cbegin() const noexcept { return data.cbegin(); }

      iterator end() noexcept { return data.end(); }
      const_iterator end() const noexcept { return data.end(); }
      const_iterator cend() const noexcept { return data.cend(); }

      reverse_iterator rbegin() noexcept { return data.rbegin(); }
      const_reverse_iterator rbegin() const noexcept { return data.rbegin(); }
      const_reverse_iterator rcbegin() const noexcept { return data.rcbegin(); }

      reverse_iterator rend() noexcept { return data.rend(); }
      const_reverse_iterator rend() const noexcept { return data.rend(); }
      const_reverse_iterator rcend() const noexcept { return data.rcend(); }

      [[nodiscard]] bool empty() const noexcept { return data.empty(); }
      size_type size() const noexcept { return data.size(); }
      size_type max_size() const noexcept { return data.max_size(); }
      size_type capacity() const noexcept { return data.capacity(); }

      /**
       * @brief Erase all elements from the container
       *
       * @details Erase all elements from the container. After this call, the size returns zero.
       * Invalidates any references, pointers, or iterators referring to contained elements. Any
       * past-the-end iterator remains valid.
       */
      void clear() noexcept { data.clear(); }

      std::pair<iterator, bool> insert(const_reference value) requires std::copyable<value_type>
      {
         if (empty())
         {
            data.push_back(value);

            return std::make_pair(begin(), true);
         }

         int32_t left = 0;
         int32_t right = size() - 1;
         while (left <= right)
         {
            int32_t midpoint = left + (right - left) / 2;
            if (key_compare comp{}; comp(data[midpoint].first, value.first))
            {
               left = midpoint + 1;
            }
            else if (data[midpoint].first == value.first)
            {
               return std::make_pair(iterator{begin() + midpoint}, false);
            }
            else
            {
               right = midpoint - 1;
            }
         }

         return std::make_pair(data.insert(cbegin() + left, value), true);
      }

      std::pair<iterator, bool> insert(value_type&& value) requires std::movable<value_type>
      {
         if (empty())
         {
            data.push_back(std::move(value));

            return std::make_pair(begin(), true);
         }

         int32_t left = 0;
         int32_t right = size() - 1;
         while (left <= right)
         {
            int32_t midpoint = left + (right - left) / 2;
            if (key_compare comp{}; comp(data[midpoint].first, value.first))
            {
               left = midpoint + 1;
            }
            else if (data[midpoint].first == value.first)
            {
               return std::make_pair(iterator{begin() + midpoint}, false);
            }
            else
            {
               right = midpoint - 1;
            }
         }

         return std::make_pair(data.insert(cbegin() + left, std::move(value)), true);
      }

      // clang-format off
      template <class value_>
      std::pair<iterator, bool> insert(value_&& value) 
         requires std::constructible_from<value_type, value_&&>
      {
         return emplace(std::forward<value_>(value));
      }
      // clang-format on

      template <std::input_iterator it_>
      void insert(it_ first, it_ last)
      {
         for (; first != last; ++first)
         {
            insert(*first);
         }
      }

      void insert(std::initializer_list<value_type> init) { insert(init.begin(), init.end()); }

      template <class... args_>
      std::pair<iterator, bool> emplace(
         args_&&... args) requires std::constructible_from<value_type, args_...>
      {
         if (empty())
         {
            data.emplace_back(std::forward<args_>(args)...);

            return std::make_pair(begin(), true);
         }

         const value_type value(std::forward<args_>(args)...);

         int32_t left = 0;
         int32_t right = size() - 1;
         while (left <= right)
         {
            const int32_t midpoint = left + (right - left) / 2;
            if (key_compare comp{}; comp(data[midpoint].first, value.first))
            {
               left = midpoint + 1;
            }
            else if (data[midpoint].first == value.first)
            {
               return std::make_pair(iterator{begin() + midpoint}, false);
            }
            else
            {
               right = midpoint - 1;
            }
         }

         return std::make_pair(data.emplace(cbegin() + left, std::move(value)), true);
      }

      template <class... args_>
      std::pair<iterator, bool> try_emplace(const key_type& key,
         args_&&... args) requires std::constructible_from<mapped_type, args_...>
      {
         if (empty())
         {
            data.emplace_back(std::make_pair(key, mapped_type(std::forward<args_>(args)...)));

            return std::make_pair(begin(), true);
         }

         int32_t left = 0;
         int32_t right = size() - 1;
         while (left <= right)
         {
            const int32_t midpoint = left + (right - left) / 2;
            if (key_compare comp{}; comp(data[midpoint].first, key))
            {
               left = midpoint + 1;
            }
            else if (data[midpoint].first == key)
            {
               return std::make_pair(iterator{begin() + midpoint}, false);
            }
            else
            {
               right = midpoint - 1;
            }
         }

         return std::make_pair(data.emplace(cbegin() + left,
                                  std::make_pair(key, mapped_type(std::forward<args_>(args)...))),
            true);
      }

      /**
       * @brief Removes specified elements from the container
       *
       * @details Remove the element at pos. References and iterators to the erased elements are
       * invalidated. other referreferences are not affected.
       *
       * @param[in] pos Iterator to the element to remove
       *
       * @return The iterator to the following element
       */
      iterator erase(const_iterator pos) { return data.erase(pos); }
      /**
       * @brief Removes specified elements from the container
       *
       * @details Removes the elements in the range [first, last), which but be a valid range.
       * References and iterators to the erased elements are invalidated. other referreferences are
       * not affected.
       *
       * @param[in] first The first element in the range to remove
       * @param[in] last The last element exclusive in the range to remove
       *
       * @return The iterator to the following element
       */
      iterator erase(const_iterator first, const_iterator last) { return data.erase(first, last); }

      size_type count(const key_type& key) const
      {
         if (find(key) != cend())
         {
            return 1;
         }
         else
         {
            return 0;
         }
      }

      iterator find(const key_type& key)
      {
         int32_t left = 0;
         int32_t right = size() - 1;
         while (left <= right)
         {
            int32_t midpoint = left + (right - left) / 2;
            if (key_compare comp{}; comp(data[midpoint].first, key))
            {
               left = midpoint + 1;
            }
            else if (data[midpoint].first == key)
            {
               return begin() + midpoint;
            }
            else
            {
               right = midpoint - 1;
            }
         }

         return end();
      }

      const_iterator find(const key_type& key) const
      {
         int32_t left = 0;
         int32_t right = size() - 1;
         while (left <= right)
         {
            int32_t midpoint = left + (right - left) / 2;
            if (key_compare comp{}; comp(data[midpoint].first, key))
            {
               left = midpoint + 1;
            }
            else if (data[midpoint].first == key)
            {
               return cbegin() + midpoint;
            }
            else
            {
               right = midpoint - 1;
            }
         }

         return cend();
      }

      bool contains(const key_type& key) const { return find(key) != cend(); }

   private:
      container_type data;
   };
} // namespace core
