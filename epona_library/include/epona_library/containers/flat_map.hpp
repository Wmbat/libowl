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
#include <epona_library/containers/dynamic_array.hpp>
#include <epona_library/utils/concepts.hpp>
#include <epona_library/utils/error_handling.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>
#include <utility>

namespace ESL
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

      mapped_type& at(key_type const& key)
      {
         auto it = find(key);
         if (it == end())
         {
            handle_out_of_range_error("key is not present in container");
         }

         return it->second;
      }
      mapped_type const& at(key_type const& key) const
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

      template <class value_>
      std::pair<iterator, bool> insert(value_&& value) requires std::constructible_from<value_type, value_&&>
      {
         return emplace(std::forward<value_>(value));
      }

      template <std::input_iterator it_>
      void insert(it_ first, it_ last)
      {
         for (auto const& val : std::span(first, last))
         {
            insert(val);
         }
      }

      void insert(std::initializer_list<value_type> init) { insert(init.begin(), init.end()); }

      template <class... args_>
      std::pair<iterator, bool> emplace(args_&&... args) requires std::constructible_from<value_type, args_...>
      {
         if (empty())
         {
            data.emplace_back(std::forward<args_>(args)...);

            return std::make_pair(begin(), true);
         }

         value_type const value(std::forward<args_>(args)...);

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

      size_type count(key_type const& key) const
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

      iterator find(key_type const& key)
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

      const_iterator find(key_type const& key) const
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

      bool contains(key_type const& key) const { return find(key) != cend(); }

   private:
      container_type data;
   };
} // namespace ESL
