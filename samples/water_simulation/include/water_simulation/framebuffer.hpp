#pragma once

#include <water_simulation/core.hpp>

#include <vkn/core.hpp>

#include <util/containers/dynamic_array.hpp>

enum struct framebuffer_error
{
   failed_to_create_framebuffer
};

/**
 * Convert a framebuffer_error enum value to string.
 */
auto to_string(framebuffer_error err) -> std::string;
/**
 * Convert a framebuffer_error enum value to a util::error_t
 */
auto to_err_code(framebuffer_error err) -> util::error_t;

class framebuffer
{
public:
   /**
    * Data used for the creation of a framebuffer object.
    */
   struct create_info
   {
      vk::Device device;
      vk::RenderPass pass;

      util::dynamic_array<vk::ImageView> attachments;

      std::uint32_t width;
      std::uint32_t height;
      std::uint32_t layers;

      std::shared_ptr<util::logger> logger;
   };

   /**
    * Attempt to create a framebuffer object from user provided information. If the creation fails,
    * return a util::error_t
    */
   static auto make(create_info&& info) -> result<framebuffer>;

public:
   auto value() const -> vk::Framebuffer; // NOLINT

private:
   vk::UniqueFramebuffer m_framebuffer;

   std::uint32_t m_width;
   std::uint32_t m_height;
   std::uint32_t m_layers;
};

namespace std
{
   template <>
   struct is_error_code_enum<framebuffer_error> : true_type
   {
   };
}; // namespace std
