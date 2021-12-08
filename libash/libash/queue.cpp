#include <libash/queue.hpp>

namespace ash::inline v0
{
   namespace detail
   {
      auto is_queue_dedicated_to_type(const queue_info& q, const vk::QueueFlags& type,
                                      auto... values) -> bool
      {
         return does_queue_support_type(q, type) and (... and !does_queue_support_type(q, values));
      }
   } // namespace detail

   auto to_queue_info(std::pair<u32, const vk::QueueFamilyProperties>&& p) -> detail::queue_info
   {
      return {.flag = p.second.queueFlags, .family = p.first, .count = p.second.queueCount};
   }

   auto detail::does_queue_support_type(const queue_info& q, const vk::QueueFlags& type) -> bool
   {
      return (q.flag & type) == type;
   }
   auto detail::does_queue_support_graphics(const queue_info& q) -> bool
   {
      return does_queue_support_type(q, vk::QueueFlagBits::eGraphics);
   }
   auto detail::does_queue_support_transfer(const queue_info& q) -> bool
   {
      return does_queue_support_type(q, vk::QueueFlagBits::eTransfer);
   }
   auto detail::does_queue_support_compute(const queue_info& q) -> bool
   {
      return does_queue_support_type(q, vk::QueueFlagBits::eCompute);
   }

   auto detail::is_queue_dedicated_to_graphics(const queue_info& q) -> bool
   {
      return is_queue_dedicated_to_type(q, vk::QueueFlagBits::eGraphics,
                                        vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eTransfer);
   }
   auto detail::is_queue_dedicated_to_transfer(const queue_info& q) -> bool
   {
      return is_queue_dedicated_to_type(q, vk::QueueFlagBits::eTransfer,
                                        vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics);
   }
   auto detail::is_queue_dedicated_to_compute(const queue_info& q) -> bool
   {
      return is_queue_dedicated_to_type(q, vk::QueueFlagBits::eCompute,
                                        vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics);
   }
} // namespace ash::inline v0
