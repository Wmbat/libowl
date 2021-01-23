#pragma once

#include <cacao/device.hpp>

#include <libcaramel/containers/dynamic_array.hpp>

namespace cacao
{
   class swapchain
   {
      static constexpr std::size_t expected_image_count = 3U;

   public:
   private:
      vk::UniqueSwapchainKHR m_swapchain{nullptr};
      crl::small_dynamic_array<vk::Image, expected_image_count> m_images;
      crl::small_dynamic_array<vk::UniqueImageView, expected_image_count> m_image_views;

      vk::Format m_format{};
      vk::Extent2D m_extent{};
   };
} // namespace cacao
