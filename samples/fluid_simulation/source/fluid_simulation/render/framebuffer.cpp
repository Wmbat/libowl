#include <fluid_simulation/render/framebuffer.hpp>

framebuffer::framebuffer(create_info&& info) :
   m_width{info.width}, m_height{info.height}, m_layers{info.layers}
{
   const auto create_info =
      vk::FramebufferCreateInfo{}
         .setPNext(nullptr)
         .setFlags({})
         .setRenderPass(info.pass)
         .setAttachmentCount(static_cast<std::uint32_t>(info.attachments.size()))
         .setPAttachments(info.attachments.data())
         .setWidth(m_width)
         .setHeight(m_height)
         .setLayers(m_layers);

   m_framebuffer = info.device.createFramebufferUnique(create_info);

   info.logger.info("Framebuffer of dimensions ({}, {}) with {} attachments created", m_width,
                    m_height, std::size(info.attachments));
}

auto framebuffer::value() const -> vk::Framebuffer
{
   return m_framebuffer.get();
}
