/**
 * @file details.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, April 23rd, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

namespace util::details
{
   /**
    * @struct static_array_storage details.hpp <ESL/containers/details.hpp>
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
} // namespace util::details
