#pragma once

#include <sph-simulation/core.hpp>

#include <libcacao/vulkan.hpp>

#include <libmannele/logging/log_ptr.hpp>
#include <libmannele/dimension.hpp>

/**
 * @brief Data used for the creation of a framebuffer object.
 */
struct framebuffer_create_info
{
   vk::Device device{};
   vk::RenderPass pass{};

   std::vector<vk::ImageView> attachments{};

   mannele::dimension_u32 dimensions;
   mannele::u32 layers;

   mannele::log_ptr logger;
};

class framebuffer
{
public:
   framebuffer() = default;
   framebuffer(const framebuffer_create_info& info);
   framebuffer(framebuffer_create_info&& info);

   auto value() const -> vk::Framebuffer; // NOLINT

private:
   mannele::dimension_u32 m_dimensions;
   mannele::u32 m_layers;

   vk::UniqueFramebuffer m_framebuffer;
};
