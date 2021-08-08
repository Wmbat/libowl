#pragma once

#include <libcacao/device.hpp>

#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <string>
#include <type_traits>

namespace cacao
{
   enum struct image_error
   {
      failed_to_find_supported_format,
      failed_to_create_image,
      failed_to_find_memory_type,
      failed_to_allocate_device_memory,
      failed_to_create_image_view
   };

   auto to_string(image_error err) -> std::string;
   auto to_err_cond(image_error err) -> error_t;

   auto find_memory_type(uint32_t type_filter, const vk::MemoryPropertyFlags& properties,
                         vk::PhysicalDevice device) -> reglisse::maybe<std::uint32_t>;
   /**
    * @brief
    *
    * @param candidates The desired format candidates
    * @param tiling The tiling the format should support
    * @param features The features the format should support
    * @param device
    *
    * @return
    */
   auto find_supported_formats(std::span<const vk::Format> candidates, vk::ImageTiling tiling,
                               const vk::FormatFeatureFlags& features, const cacao::device& device)
      -> reglisse::maybe<vk::Format>;
   auto find_depth_format(const cacao::device& device) -> reglisse::maybe<vk::Format>;
   auto find_colour_format(const cacao::device& device) -> reglisse::maybe<vk::Format>;

   struct image_create_info
   {
      util::log_ptr logger;

      cacao::device& device;

      std::vector<vk::Format> formats;

      vk::ImageTiling tiling;
      vk::ImageUsageFlags usage;
      vk::MemoryPropertyFlags memory_properties;

      std::uint32_t width{0};
      std::uint32_t height{0};
   };

   class image
   {
   public:
      image() = default;
      image(image_create_info&& info) :
         m_tiling{info.tiling},
         m_subresource_range{.aspectMask = to_image_aspect_flag(info.usage),
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1},
         m_usage{info.usage},
         m_memory_properties{info.memory_properties},
         m_width{info.width},
         m_height{info.height}
      {
         auto fmt_feature_flag = to_format_feature_flags(m_usage);

         const auto fmt =
            find_supported_formats(info.formats, m_tiling, fmt_feature_flag, info.device);

         if (!fmt)
         {
            throw cacao::runtime_error{to_err_cond(image_error::failed_to_find_supported_format)};
         }

         auto logical = info.device.logical();
         auto physical = info.device.physical();

         m_fmt = fmt.borrow();
         m_image = create_image(logical);
         m_memory = allocate_memory(logical, physical);

         logical.bindImageMemory(m_image.get(), m_memory.get(), 0);

         m_view = create_image_view(logical);

         info.logger.info(
            "image of dimensions ({}, {}) using {} memory for {} with format {} created", m_width,
            m_height, vk::to_string(info.memory_properties), vk::to_string(m_usage),
            vk::to_string(m_fmt));
      }

      /**
       * @brief Access the underlying vulkan image handle
       *
       * @return The handle to the vulkan image.
       */
      auto value() const -> vk::Image { return m_image.get(); } // NOLINT
      /**
       * @brief Access the underlying vulkan image view  handle
       *
       * @return The handle to the vulkan image view.
       */
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
      [[nodiscard]] auto create_image(vk::Device device) const -> vk::UniqueImage
      {
         return device.createImageUnique(
            {.imageType = vk::ImageType::e2D,
             .format = m_fmt,
             .extent = {.width = m_width, .height = m_height, .depth = 1},
             .mipLevels = 1,
             .arrayLayers = 1,
             .samples = vk::SampleCountFlagBits::e1,
             .tiling = m_tiling,
             .usage = m_usage,
             .sharingMode = vk::SharingMode::eExclusive,
             .initialLayout = vk::ImageLayout::eUndefined});
      }
      [[nodiscard]] auto allocate_memory(vk::Device device, vk::PhysicalDevice physical) const
         -> vk::UniqueDeviceMemory
      {
         const auto memory_requirements = device.getImageMemoryRequirements(m_image.get());
         const auto memory_type_index =
            find_memory_type(memory_requirements.memoryTypeBits, m_memory_properties, physical);

         if (memory_type_index)
         {
            return device.allocateMemoryUnique({.allocationSize = memory_requirements.size,
                                                .memoryTypeIndex = memory_type_index.borrow()});
         }

         throw cacao::runtime_error{to_err_cond(image_error::failed_to_find_memory_type)};
      }
      [[nodiscard]] auto create_image_view(vk::Device device) const -> vk::UniqueImageView
      {
         auto t = vk::ImageViewCreateInfo{.image = m_image.get(),
                                          .viewType = vk::ImageViewType::e2D,
                                          .format = m_fmt,
                                          .subresourceRange = m_subresource_range};

         return device.createImageViewUnique(t);
      }

      [[nodiscard]] auto to_format_feature_flags(const vk::ImageUsageFlags& flags) const noexcept
         -> vk::FormatFeatureFlagBits
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

      [[nodiscard]] auto to_image_aspect_flag(const vk::ImageUsageFlags& flags) const noexcept
         -> vk::ImageAspectFlagBits
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

   private:
      vk::UniqueImage m_image;
      vk::UniqueDeviceMemory m_memory;
      vk::UniqueImageView m_view;

      vk::Format m_fmt{};
      vk::ImageTiling m_tiling{};
      vk::ImageSubresourceRange m_subresource_range;
      vk::ImageUsageFlags m_usage;
      vk::MemoryPropertyFlags m_memory_properties;

      std::uint32_t m_width{0};
      std::uint32_t m_height{0};
   };

   static constexpr std::array depth_formats = {
      vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
   static constexpr std::array colour_formats = {vk::Format::eR8G8B8A8Srgb};
} // namespace cacao
