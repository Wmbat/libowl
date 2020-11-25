#pragma once

#include <water_simulation/core.hpp>

#include <vkn/core.hpp>

#include <util/containers/dynamic_array.hpp>

enum struct framebuffer_error
{
   failed_to_create_framebuffer
};

/**
 * @brief Convert a `framebuffer_error` enum value to `std::string`.
 *
 * @param err The error to convert
 *
 * @return The string representation of the `framebuffer_error` passed as parameter
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
    * @brief Data used for the creation of a framebuffer object.
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
    * @brief Construct a `framebuffer` object using data provided through the `create_info` struct.
    *
    * @param info The information needed for the creation of a `framebuffer` object.
    *
    * @return A `result` holding one of two things:
    * * A `util::error_t` holding relevant information about reason behind the failure to construct
    * the framebuffer object
    * * A fully constructed `framebuffer` object
    */
   static auto make(create_info&& info) -> result<framebuffer>;

public:
   /**
    * @brief access the underlying vulkan handle
    *
    * @return The vulkan `vk::framebuffer` handle.
    */
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
