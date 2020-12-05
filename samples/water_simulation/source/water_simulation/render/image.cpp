#include <water_simulation/render/image.hpp>

namespace detail
{
   auto create_image(image_data&& data) -> result<image_data>
   {
      vk::ImageCreateInfo create_info{
         .imageType = vk::ImageType::e2D,
         .format = data.format,
         .extent = {.width = data.info.width, .height = data.info.height, .depth = 1},
         .mipLevels = 1,
         .arrayLayers = 1,
         .samples = vk::SampleCountFlagBits::e1,
         .tiling = data.tiling,
         .usage = data.usage,
         .sharingMode = vk::SharingMode::eExclusive,
         .initialLayout = vk::ImageLayout::eUndefined};

      return monad::try_wrap<vk::SystemError>([&] {
                return data.info.device.logical().createImageUnique(create_info);
             })
         .map_error([](auto /*err*/) {
            return to_err_code(image_error::failed_to_create_image);
         })
         .map([&](vk::UniqueImage&& image) {
            data.image = std::move(image);
            return std::move(data);
         });
   }
   auto allocate_memory(image_data&& data) -> result<image_data>
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
                  return to_err_code(image_error::failed_to_allocate_device_memory);
               });
         })
         .map([&](vk::UniqueDeviceMemory&& mem) {
            data.memory = std::move(mem);

            device.bindImageMemory(data.image.get(), data.memory.get(), 0);

            return std::move(data);
         });
   }
   auto create_image_view(image_data&& data) -> result<image_data>
   {
      const auto device = data.info.device.logical();
      const vk::ImageViewCreateInfo create_info{.image = data.image.get(),
                                                .viewType = vk::ImageViewType::e2D,
                                                .format = data.format,
                                                .subresourceRange = data.subresource_range};

      return monad::try_wrap<vk::SystemError>([&] {
                return device.createImageViewUnique(create_info);
             })
         .map_error([](auto /*err*/) {
            return to_err_code(image_error::failed_to_create_image_view);
         })
         .map([&](vk::UniqueImageView&& view) {
            data.view = std::move(view);
            return std::move(data);
         });
   }
} // namespace detail

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

   return monad::err(to_err_code(image_error::failed_to_find_memory_type));
}
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

   return monad::err(to_err_code(image_error::failed_to_find_supported_format));
}
auto find_depth_format(const vkn::device& device) -> result<vk::Format>
{
   return find_supported_formats(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment, device);
}
auto find_colour_format(const vkn::device& device) -> result<vk::Format>
{
   return find_supported_formats({vk::Format::eR8G8B8A8Srgb}, vk::ImageTiling::eOptimal,
                                 vk::FormatFeatureFlagBits::eColorAttachment, device);
}

struct image_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "depth_buffer"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return to_string(static_cast<image_error>(err));
   }
};

static const image_error_category image_error_cat{};

auto to_string(image_error err) -> std::string
{
   if (err == image_error::failed_to_find_supported_format)
   {
      return "failed_to_find_supported_format";
   }

   if (err == image_error::failed_to_create_image)
   {
      return "failed_to_create_image";
   }

   if (err == image_error::failed_to_find_memory_type)
   {
      return "failed_to_find_memory_type";
   }

   if (err == image_error::failed_to_allocate_device_memory)
   {
      return "failed_to_allocate_device_memory";
   }

   if (err == image_error::failed_to_create_image_view)
   {
      return "failed_to_create_image_view";
   }

   return "UNKNOWN";
}
auto to_err_code(image_error err) -> util::error_t
{
   return {{static_cast<int>(err), image_error_cat}};
}
