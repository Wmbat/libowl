#pragma once

#include <vkn/sync/fence.hpp>

namespace vkn
{
   class fence_observer final
   {
   public:
      using value_type = vk::Fence;
      using pointer = vk::Fence*;
      using const_pointer = const vk::Fence*;

      constexpr fence_observer() = default;
      constexpr fence_observer(value_type value) noexcept : m_value{value} {}
      constexpr fence_observer(const fence& fence) noexcept : m_value{fence.value()} {}

      constexpr auto operator->() noexcept -> pointer { return &m_value; }
      constexpr auto operator->() const noexcept -> const_pointer { return &m_value; }

      constexpr auto operator*() const noexcept -> value_type { return value(); }

      [[nodiscard]] constexpr auto value() const noexcept -> value_type { return m_value; }

   private:
      value_type m_value{nullptr};
   };
} // namespace vkn
