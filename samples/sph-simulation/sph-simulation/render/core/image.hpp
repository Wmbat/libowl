#ifndef SPH_SIMULATION_RENDER_IMAGE_HPP
#define SPH_SIMULATION_RENDER_IMAGE_HPP

#include <libcacao/device.hpp>

#include <libmannele/dimension.hpp>

struct image_create_info
{
   const cacao::device& device;

   std::vector<vk::Format> formats;

   vk::ImageTiling tiling;
   vk::ImageUsageFlags usage;
   vk::MemoryPropertyFlags memory_properties;

   mannele::dimension_u32 dimensions;

   util::log_ptr logger;
};

class image
{
public:
   image() = default;
   image(const image_create_info& info);

   [[nodiscard]] auto value() const noexcept -> vk::Image;
   [[nodiscard]] auto view() const noexcept -> vk::ImageView;
   [[nodiscard]] auto dimensions() const noexcept -> const mannele::dimension_u32&;
   [[nodiscard]] auto subresource_layers() const -> vk::ImageSubresourceLayers;

private:
   vk::UniqueImage m_image;
   vk::UniqueDeviceMemory m_memory;
   vk::UniqueImageView m_view;

   vk::Format m_fmt{};
   vk::ImageTiling m_tiling{};
   vk::ImageSubresourceRange m_subresource_range;
   vk::ImageUsageFlags m_usage;
   vk::MemoryPropertyFlags m_memory_properties;

   mannele::dimension_u32 m_dimensions{};
};

auto find_depth_format(const cacao::device& device) -> reglisse::maybe<vk::Format>;
auto find_colour_format(const cacao::device& device) -> reglisse::maybe<vk::Format>;

[[maybe_unused]] static constexpr std::array depth_formats = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
                                             vk::Format::eD24UnormS8Uint};
[[maybe_unused]] static constexpr std::array colour_formats = {vk::Format::eR8G8B8A8Srgb};

#endif // SPH_SIMULATION_RENDER_IMAGE_HPP
