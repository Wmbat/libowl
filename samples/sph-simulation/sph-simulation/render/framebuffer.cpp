#include <sph-simulation/render/framebuffer.hpp>

framebuffer::framebuffer(const framebuffer_create_info& info) :
   m_dimensions(info.dimensions), m_layers(info.layers),
   m_framebuffer(info.device.createFramebufferUnique(
      {.renderPass = info.pass,
       .attachmentCount = static_cast<mannele::u32>(std::size(info.attachments)),
       .pAttachments = std::data(info.attachments),
       .width = m_dimensions.width,
       .height = m_dimensions.height,
       .layers = m_layers}))
{
   auto logger = info.logger;
   logger.debug("Framebuffer of dimensions ({}, {}) with {} attachments created", m_dimensions.width,
               m_dimensions.height, std::size(info.attachments));
}
framebuffer::framebuffer(framebuffer_create_info&& info) :
   m_dimensions(info.dimensions), m_layers(info.layers),
   m_framebuffer(info.device.createFramebufferUnique(vk::FramebufferCreateInfo{
      .renderPass = info.pass,
      .attachmentCount = static_cast<mannele::u32>(std::size(info.attachments)),
      .pAttachments = std::data(info.attachments),
      .width = m_dimensions.width,
      .height = m_dimensions.height,
      .layers = m_layers}))
{
   info.logger.debug("Framebuffer of dimensions ({}, {}) with {} attachments created",
                    m_dimensions.width, m_dimensions.height, std::size(info.attachments));
}

auto framebuffer::value() const -> vk::Framebuffer
{
   return m_framebuffer.get();
}
