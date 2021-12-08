/**
 * @file libash/queue.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBASH_QUEUE_HPP_
#define LIBASH_QUEUE_HPP_

#include <libash/core.hpp>
#include <libash/detail/vulkan.hpp>

namespace ash::inline v0
{
   struct queue
   {
      vk::Queue value;
      vk::QueueFlags type{};

      u32 family_index{};
      u32 queue_index{};
   };

   namespace detail
   {
      struct queue_info
      {
         vk::QueueFlags flag;
         u32 family;
         u32 count;
      };

      auto to_queue_info(std::pair<u32, const vk::QueueFamilyProperties>&& p) -> detail::queue_info;

      auto does_queue_support_type(const queue_info& q, const vk::QueueFlags& type) -> bool;
      auto does_queue_support_graphics(const queue_info& q) -> bool;
      auto does_queue_support_transfer(const queue_info& q) -> bool;
      auto does_queue_support_compute(const queue_info& q) -> bool;

      auto is_queue_dedicated_to_graphics(const queue_info& q) -> bool;
      auto is_queue_dedicated_to_transfer(const queue_info& q) -> bool;
      auto is_queue_dedicated_to_compute(const queue_info& q) -> bool;
   } // namespace detail
} // namespace ash::inline v0

#endif // LIBASH_QUEUE_HPP_
