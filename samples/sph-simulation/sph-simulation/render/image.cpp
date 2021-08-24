#include <sph-simulation/render/image.hpp>

using namespace reglisse;

auto to_format_feature_flags(const vk::ImageUsageFlags& flags) noexcept -> vk::FormatFeatureFlagBits
{
   if ((flags & vk::ImageUsageFlagBits::eColorAttachment) ==
       vk::ImageUsageFlagBits::eColorAttachment)
   {
      return vk::FormatFeatureFlagBits::eColorAttachment;
   }

   if ((flags & vk::ImageUsageFlagBits::eDepthStencilAttachment) ==
       vk::ImageUsageFlagBits::eDepthStencilAttachment)
   {
      return vk::FormatFeatureFlagBits::eDepthStencilAttachment;
   }

   return {};
}

auto find_supported_formats(std::span<const vk::Format> candidates, vk::ImageTiling tiling,
                            const vk::FormatFeatureFlags& features, const cacao::device& device)
   -> maybe<vk::Format>
{
   for (vk::Format format : candidates)
   {
      const auto properties = device.physical().getFormatProperties(format);

      if (tiling == vk::ImageTiling::eLinear &&
          (properties.linearTilingFeatures & features) == features)
      {
         return some(format);
      }

      if (tiling == vk::ImageTiling::eOptimal &&
          (properties.optimalTilingFeatures & features) == features)
      {
         return some(format);
      }
   }

   return none;
}

auto find_memory_type(uint32_t type_filter, const vk::MemoryPropertyFlags& properties,
                      vk::PhysicalDevice device) -> maybe<std::uint32_t>
{
   const auto memory_properties = device.getMemoryProperties();

   for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
   {
      if ((type_filter & (1 << i)) &&
          (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
      {
         return some(i);
      }
   }

   return none;
}

auto to_image_aspect_flag(const vk::ImageUsageFlags& flags) noexcept -> vk::ImageAspectFlagBits
{
   if ((flags & vk::ImageUsageFlagBits::eColorAttachment) ==
       vk::ImageUsageFlagBits::eColorAttachment)
   {
      return vk::ImageAspectFlagBits::eColor;
   }

   if ((flags & vk::ImageUsageFlagBits::eDepthStencilAttachment) ==
       vk::ImageUsageFlagBits::eDepthStencilAttachment)
   {
      return vk::ImageAspectFlagBits::eDepth;
   }

   return {};
}

image::image(const image_create_info& info) :
   m_tiling(info.tiling), m_subresource_range{.aspectMask = to_image_aspect_flag(info.usage),
                                              .baseMipLevel = 0,
                                              .levelCount = 1,
                                              .baseArrayLayer = 0,
                                              .layerCount = 1},
   m_usage(info.usage), m_memory_properties(info.memory_properties), m_dimensions(info.dimensions)
{
   const auto logical = info.device.logical();
   const auto physical = info.device.physical();
   const auto format_feature_flag = to_format_feature_flags(m_usage);

   if (auto fmt = find_supported_formats(info.formats, m_tiling, format_feature_flag, info.device))
   {
      m_fmt = fmt.borrow();
   }
   else
   {
      // TODO: Actually handle
      throw cacao::runtime_error({});
   }

   m_image = logical.createImageUnique(
      {.imageType = vk::ImageType::e2D,
       .format = m_fmt,
       .extent = {.width = m_dimensions.width, .height = m_dimensions.height, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .samples = vk::SampleCountFlagBits::e1,
       .tiling = m_tiling,
       .usage = m_usage,
       .sharingMode = vk::SharingMode::eExclusive,
       .initialLayout = vk::ImageLayout::eUndefined});

   const auto memory_reqs = logical.getImageMemoryRequirements(m_image.get());
   if (auto type = find_memory_type(memory_reqs.memoryTypeBits, m_memory_properties, physical))
   {
      m_memory = logical.allocateMemoryUnique(
         {.allocationSize = memory_reqs.size, .memoryTypeIndex = type.borrow()});
   }
   else
   {
      throw cacao::runtime_error({});
   }

   logical.bindImageMemory(m_image.get(), m_memory.get(), 0);

   m_view = logical.createImageViewUnique({.image = m_image.get(),
                                           .viewType = vk::ImageViewType::e2D,
                                           .format = m_fmt,
                                           .subresourceRange = m_subresource_range});
}

auto image::value() const noexcept -> vk::Image
{
   return m_image.get();
}
auto image::view() const noexcept -> vk::ImageView
{
   return m_view.get();
}
auto image::dimensions() const noexcept -> const mannele::dimension_u32&
{
   return m_dimensions;
}
auto image::subresource_layers() const -> vk::ImageSubresourceLayers
{
   return {.aspectMask = m_subresource_range.aspectMask,
           .mipLevel = m_subresource_range.baseMipLevel,
           .baseArrayLayer = m_subresource_range.baseArrayLayer,
           .layerCount = m_subresource_range.layerCount};
};

auto find_depth_format(const cacao::device& device) -> reglisse::maybe<vk::Format>
{
   return find_supported_formats(depth_formats, vk::ImageTiling::eOptimal,
                                 vk::FormatFeatureFlagBits::eDepthStencilAttachment, device);
}
auto find_colour_format(const cacao::device& device) -> reglisse::maybe<vk::Format>
{
   return find_supported_formats(colour_formats, vk::ImageTiling::eOptimal,
                                 vk::FormatFeatureFlagBits::eColorAttachment, device);
}
