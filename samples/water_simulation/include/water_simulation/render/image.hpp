#pragma once

#include <water_simulation/core.hpp>

#include <vkn/device.hpp>

enum struct image_error
{
   failed_to_find_supported_format,
   failed_to_create_image,
   failed_to_find_memory_type,
   failed_to_allocate_device_memory,
   failed_to_create_image_view
};

auto to_string(image_error err) -> std::string;
auto to_err_code(image_error err) -> util::error_t;

enum struct image_type
{
   colour,
   depth
};

struct image_create_info
{
   vkn::device& device;

   std::uint32_t width{0};
   std::uint32_t height{0};
};

namespace detail
{
   struct image_data
   {
      image_create_info& info;

      vk::Format format;
      vk::ImageTiling tiling;
      vk::ImageUsageFlags usage;
      vk::MemoryPropertyFlags properties;
      vk::ImageSubresourceRange subresource_range;

      vk::UniqueImage image;
      vk::UniqueDeviceMemory memory;
      vk::UniqueImageView view;
   };

   auto create_image(image_data&& data) -> result<image_data>;
   auto allocate_memory(image_data&& data) -> result<image_data>;
   auto create_image_view(image_data&& data) -> result<image_data>;
} // namespace detail

auto find_memory_type(uint32_t type_filter, const vk::MemoryPropertyFlags& properties,
                      const vkn::device& device) -> result<uint32_t>;
auto find_supported_formats(const util::dynamic_array<vk::Format>& candidates,
                            vk::ImageTiling tiling, const vk::FormatFeatureFlags& features,
                            const vkn::device& device) -> result<vk::Format>;
auto find_depth_format(const vkn::device& device) -> result<vk::Format>;
auto find_colour_format(const vkn::device& device) -> result<vk::Format>;

template <image_type Val>
class image
{
public:
   static auto make(image_create_info&& info) -> result<image>
   {
      auto finalize = [](detail::image_data&& data) {
         image img;
         img.m_image = std::move(data.image);
         img.m_memory = std::move(data.memory);
         img.m_view = std::move(data.view);
         img.m_width = data.info.width;
         img.m_height = data.info.height;
         img.m_subresource_range = data.subresource_range;

         return img;
      };

      if constexpr (Val == image_type::colour)
      {
         return find_colour_format(info.device)
            .and_then([&](vk::Format format) {
               return detail::create_image(
                  {.info = info,
                   .format = format,
                   .usage = vk::ImageUsageFlagBits::eColorAttachment |
                      vk::ImageUsageFlagBits::eTransferSrc,
                   .subresource_range = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                         .baseMipLevel = 0,
                                         .levelCount = 1,
                                         .baseArrayLayer = 0,
                                         .layerCount = 1}});
            })
            .and_then(detail::allocate_memory)
            .and_then(detail::create_image_view)
            .map(finalize);
      }

      if constexpr (Val == image_type::depth)
      {
         return find_depth_format(info.device)
            .and_then([&](vk::Format format) {
               return detail::create_image(
                  {.info = info,
                   .format = format,
                   .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                   .subresource_range = {.aspectMask = vk::ImageAspectFlagBits::eDepth,
                                         .baseMipLevel = 0,
                                         .levelCount = 1,
                                         .baseArrayLayer = 0,
                                         .layerCount = 1}});
            })
            .and_then(detail::allocate_memory)
            .and_then(detail::create_image_view)
            .map(finalize);
      }
   }

public:
   auto value() const -> vk::Image { return m_image.get(); }     // NOLINT
   auto view() const -> vk::ImageView { return m_view.get(); }   // NOLINT
   auto subresource_layers() const -> vk::ImageSubresourceLayers // NOLINT
   {
      return {.aspectMask = m_subresource_range.aspectMask,
              .mipLevel = m_subresource_range.baseMipLevel,
              .baseArrayLayer = m_subresource_range.baseArrayLayer,
              .layerCount = m_subresource_range.layerCount};
   };
   auto width() const -> std::uint32_t { return m_width; }   // NOLINT
   auto height() const -> std::uint32_t { return m_height; } // NOLINT

private:
   vk::UniqueImage m_image;
   vk::UniqueDeviceMemory m_memory;
   vk::UniqueImageView m_view;

   vk::ImageSubresourceRange m_subresource_range;

   std::uint32_t m_width{0};
   std::uint32_t m_height{0};
};

using colour_image = image<image_type::colour>;
using depth_image = image<image_type::depth>;
