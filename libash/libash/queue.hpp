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

#include <fmt/core.h>

namespace ash::inline v0
{
   /**
    * @brief
    */
   struct queue
   {
      vk::Queue value;       ///< The vulkan queue handle
      vk::QueueFlags type{}; ///< The operations supported by the queue
      u32 family_index{};    ///< The queue's family
      u32 queue_index{};     ///< The queue's index in the family

      friend auto operator==(queue const& lhs, queue const& rhs) noexcept -> bool = default;
   };
} // namespace ash::inline v0

template <>
struct fmt::formatter<ash::queue>
{
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

   template <typename FormatContext>
   auto format(const ash::queue& queue, FormatContext& ctx)
   {
      return format_to(ctx.out(),
                       "{{.value = {}, .type = {}, .family_index = {}, .queue_index = {}}}",
                       static_cast<void*>(static_cast<VkQueue>(queue.value)),
                       vk::to_string(queue.type), queue.family_index, queue.queue_index);
   }
};

#endif // LIBASH_QUEUE_HPP_
