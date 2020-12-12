#include <water_simulation/render/image.hpp>

#include <ostream>
#include <ranges>

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
auto to_err_cond(image_error err) -> vml::error_t
{
   return {{static_cast<int>(err), image_error_cat}};
}

auto find_memory_type(uint32_t type_filter, const vk::MemoryPropertyFlags& properties,
                      vk::PhysicalDevice device) -> monad::maybe<std::uint32_t>
{
   const auto memory_properties = device.getMemoryProperties();

   for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
   {
      if ((type_filter & (1 << i)) &&
          (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
      {
         return i;
      }
   }

   return monad::none;
}
auto find_supported_formats(std::span<const vk::Format> candidates, vk::ImageTiling tiling,
                            const vk::FormatFeatureFlags& features, const vkn::device& device)
   -> monad::maybe<vk::Format>
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

   return monad::none;
}
auto find_depth_format(const vkn::device& device) -> monad::maybe<vk::Format>
{
   return find_supported_formats(depth_formats, vk::ImageTiling::eOptimal,
                                 vk::FormatFeatureFlagBits::eDepthStencilAttachment, device);
}
auto find_colour_format(const vkn::device& device) -> monad::maybe<vk::Format>
{
   return find_supported_formats(colour_formats, vk::ImageTiling::eOptimal,
                                 vk::FormatFeatureFlagBits::eColorAttachment, device);
}

auto to_string(image_flags flags) -> std::string
{
   util::dynamic_array<const char*> strs;

   if ((flags & image_flags::colour) == image_flags::colour)
   {
      strs.push_back("colour");
   }

   if ((flags & image_flags::transfer_src) == image_flags::transfer_src)
   {
      strs.push_back("transfer_src");
   }

   if ((flags & image_flags::depth_stencil) == image_flags::depth_stencil)
   {
      strs.push_back("depth_stencil");
   }

   std::ostringstream oss;
   if (!strs.empty())
   {
      oss << "{";
      std::copy(std::begin(strs), std::end(strs) - 1,
                std::ostream_iterator<std::string>(oss, ", "));
      oss << strs.back() << "}";
   }

   return oss.str();
}
