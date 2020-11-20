#include <water_simulation/render/depth_buffer.hpp>

#include <monads/try.hpp>

struct depth_buffer_data
{
   depth_buffer::create_info& info;

   vk::Format format;
   vk::ImageTiling tiling;
   vk::ImageUsageFlags usage;
   vk::MemoryPropertyFlags properties;

   vk::UniqueImage image;
   vk::UniqueDeviceMemory memory;
   vk::UniqueImageView view;
};

auto find_supported_formats(const util::dynamic_array<vk::Format>& candidates,
                            vk::ImageTiling tiling, const vk::FormatFeatureFlags& features,
                            const vkn::device& device) -> result<vk::Format>
{
   for (vk::Format format : candidates)
   {
      const auto properties = device.physical().getFormatProperties(format);

      if (tiling == vk::ImageTiling::eLinear &&
          (properties.linearTilingFeatures & features) == features)
      {
         return format;
      }

      if (tiling == vk::ImageTiling::eOptimal &&
          (properties.optimalTilingFeatures & features) == features)
      {
         return format;
      }
   }

   return monad::err(to_err_code(depth_buffer_error::failed_to_find_supported_format));
}

auto find_depth_format(const vkn::device& device) -> result<vk::Format>
{
   return find_supported_formats(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment, device);
}

auto has_stencil_component(vk::Format format) -> bool
{
   return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

auto create_image(depth_buffer_data&& data) -> result<depth_buffer_data>
{
   vk::ImageCreateInfo create_info{
      .imageType = vk::ImageType::e2D,
      .format = data.format,
      .extent = {.width = data.info.width, .height = data.info.height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = data.tiling,
      .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

   return monad::try_wrap<vk::SystemError>([&] {
             return data.info.device.logical().createImageUnique(create_info);
          })
      .map_error([](auto /*err*/) {
         return to_err_code(depth_buffer_error::failed_to_create_image);
      })
      .map([&](vk::UniqueImage&& image) {
         data.image = std::move(image);
         return std::move(data);
      });
}

auto find_memory_type(uint32_t type_filter, const vk::MemoryPropertyFlags& properties,
                      const vkn::device& device) -> result<uint32_t>
{
   const auto physical = device.physical();
   const auto memory_properties = physical.getMemoryProperties();

   for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
   {
      if ((type_filter & (1 << i)) &&
          (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
      {
         return i;
      }
   }

   return monad::err(to_err_code(depth_buffer_error::failed_to_find_memory_type));
}

auto allocate_memory(depth_buffer_data&& data) -> result<depth_buffer_data>
{
   const auto device = data.info.device.logical();

   const auto memory_requirements = device.getImageMemoryRequirements(data.image.get());

   return find_memory_type(memory_requirements.memoryTypeBits, data.properties, data.info.device)
      .and_then([&](std::uint32_t type) {
         return monad::try_wrap<vk::SystemError>([&] {
                   return device.allocateMemoryUnique(
                      {.allocationSize = memory_requirements.size, .memoryTypeIndex = type});
                })
            .map_error([](auto /*err*/) {
               return to_err_code(depth_buffer_error::failed_to_allocate_device_memory);
            });
      })
      .map([&](vk::UniqueDeviceMemory&& mem) {
         data.memory = std::move(mem);

         device.bindImageMemory(data.image.get(), data.memory.get(), 0);

         return std::move(data);
      });
}

auto create_image_view(depth_buffer_data&& data) -> result<depth_buffer_data>
{
   const auto device = data.info.device.logical();
   const vk::ImageViewCreateInfo create_info{
      .image = data.image.get(),
      .viewType = vk::ImageViewType::e2D,
      .format = data.format,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eDepth,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

   return monad::try_wrap<vk::SystemError>([&] {
             return device.createImageViewUnique(create_info);
          })
      .map_error([](auto /*err*/) {
         return to_err_code(depth_buffer_error::failed_to_create_image_view);
      })
      .map([&](vk::UniqueImageView&& view) {
         data.view = std::move(view);
         return std::move(data);
      });
}

auto depth_buffer::make(create_info&& info) -> result<depth_buffer>
{
   auto finalize = [](depth_buffer_data&& data) {
      depth_buffer buffer;
      buffer.m_image = std::move(data.image);
      buffer.m_memory = std::move(data.memory);
      buffer.m_view = std::move(data.view);
      buffer.m_width = data.info.width;
      buffer.m_height = data.info.height;
      buffer.m_format = data.format;

      return buffer;
   };

   return find_depth_format(info.device)
      .and_then([&](vk::Format format) {
         return create_image({.info = info, .format = format});
      })
      .and_then(allocate_memory)
      .and_then(create_image_view)
      .map(finalize);
}

auto depth_buffer::view() const -> vk::ImageView
{
   return m_view.get();
}

struct depth_buffer_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "depth_buffer"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return to_string(static_cast<depth_buffer_error>(err));
   }
};

static const depth_buffer_error_category depth_buffer_error_cat{};

auto to_string(depth_buffer_error err) -> std::string
{
   if (err == depth_buffer_error::failed_to_find_supported_format)
   {
      return "failed_to_find_supported_format";
   }

   if (err == depth_buffer_error::failed_to_create_image)
   {
      return "failed_to_create_image";
   }

   if (err == depth_buffer_error::failed_to_find_memory_type)
   {
      return "failed_to_find_memory_type";
   }

   if (err == depth_buffer_error::failed_to_allocate_device_memory)
   {
      return "failed_to_allocate_device_memory";
   }

   if (err == depth_buffer_error::failed_to_create_image_view)
   {
      return "failed_to_create_image_view";
   }

   return "UNKNOWN";
}
auto to_err_code(depth_buffer_error err) -> util::error_t
{
   return {{static_cast<int>(err), depth_buffer_error_cat}};
}
