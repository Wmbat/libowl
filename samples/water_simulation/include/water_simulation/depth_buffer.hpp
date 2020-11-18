#pragma once

#include <water_simulation/core.hpp>

#include <vkn/device.hpp>

enum struct depth_buffer_error
{
   failed_to_find_supported_format,
   failed_to_create_image,
   failed_to_find_memory_type,
   failed_to_allocate_device_memory,
   failed_to_create_image_view
};

auto to_string(depth_buffer_error err) -> std::string;
auto to_err_code(depth_buffer_error err) -> util::error_t;

class depth_buffer
{
public:
   struct create_info
   {
      vkn::device& device;

      std::uint32_t width;
      std::uint32_t height;
   };

   static auto make(create_info&& info) -> result<depth_buffer>;

public:
   auto view() const -> vk::ImageView; // NOLINT

private:
   vk::UniqueImage m_image;
   vk::UniqueDeviceMemory m_memory;
   vk::UniqueImageView m_view;

   std::uint32_t m_width;
   std::uint32_t m_height;

   vk::Format m_format;
};

auto find_depth_format(const vkn::device& device) -> result<vk::Format>;
