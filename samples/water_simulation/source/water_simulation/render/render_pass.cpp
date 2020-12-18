#include <water_simulation/render/render_pass.hpp>

#include <monads/try.hpp>

#include <range/v3/view/iota.hpp>

namespace vi = ranges::views;

render_pass::render_pass(render_pass_create_info&& info)
{
   ENSURE(info.device != nullptr);
   ENSURE(info.swapchain != nullptr);

   m_render_pass = create_render_pass(info);
   m_framebuffers = create_framebuffers(info);
   m_buff_calls = [](vk::CommandBuffer) {};
}

auto render_pass::value() const -> vk::RenderPass
{
   return m_render_pass.get();
}

void render_pass::record_render_calls(const std::function<void(vk::CommandBuffer)>& calls)
{
   m_buff_calls = calls;
}

void render_pass::submit_render_calls(vk::CommandBuffer cmd_buffer, util::index_t framebuffer_index,
                                      vk::Rect2D render_area,
                                      std::span<const vk::ClearValue> clear_colours)
{
   ENSURE(framebuffer_index.value() < std::size(m_framebuffers));

   cmd_buffer.beginRenderPass(
      {.pNext = nullptr,
       .renderPass = m_render_pass.get(),
       .framebuffer = m_framebuffers.lookup(framebuffer_index.value()).value(),
       .renderArea = render_area,
       .clearValueCount = static_cast<std::uint32_t>(std::size(clear_colours)),
       .pClearValues = std::data(clear_colours)},
      vk::SubpassContents::eInline);

   std::invoke(m_buff_calls, cmd_buffer);

   cmd_buffer.endRenderPass();
}

auto render_pass::create_render_pass(const render_pass_create_info& info) -> vk::UniqueRenderPass
{
   std::array<vk::AttachmentDescription, 2> attachment_descriptions{};

   if (info.colour_attachment)
   {
      attachment_descriptions[0] = info.colour_attachment.value();
   }

   if (info.depth_stencil_attachment)
   {
      attachment_descriptions[1] = info.depth_stencil_attachment.value();
   }

   const vk::AttachmentReference colour_ref{.attachment = 0,
                                            .layout = vk::ImageLayout::eColorAttachmentOptimal};
   const vk::AttachmentReference depth_stencil_ref{
      .attachment = 1, .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

   const crl::small_dynamic_array<vk::SubpassDescription, 1> descriptions{
      {.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
       .colorAttachmentCount = 1,
       .pColorAttachments = &colour_ref,
       .pDepthStencilAttachment = &depth_stencil_ref}};

   const crl::small_dynamic_array<vk::SubpassDependency, 1> dependencies{
      {.srcSubpass = VK_SUBPASS_EXTERNAL,
       .dstSubpass = 0,
       .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
          vk::PipelineStageFlagBits::eEarlyFragmentTests,
       .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
          vk::PipelineStageFlagBits::eEarlyFragmentTests,
       .srcAccessMask = {},
       .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
          vk::AccessFlagBits::eDepthStencilAttachmentWrite}};

   return info.device.createRenderPassUnique(
      {.attachmentCount = static_cast<std::uint32_t>(std::size(attachment_descriptions)),
       .pAttachments = std::data(attachment_descriptions),
       .subpassCount = static_cast<std::uint32_t>(std::size(descriptions)),
       .pSubpasses = std::data(descriptions),
       .dependencyCount = static_cast<std::uint32_t>(std::size(dependencies)),
       .pDependencies = std::data(dependencies)});
}
auto render_pass::create_framebuffers(const render_pass_create_info& info)
   -> crl::dynamic_array<framebuffer>
{
   auto framebuffer_infos = info.framebuffer_create_infos;
   const std::size_t framebuffer_count = std::size(framebuffer_infos);

   crl::dynamic_array<framebuffer> framebuffers;
   framebuffers.reserve(framebuffer_count);

   for (auto i : vi::iota(0u, framebuffer_count))
   {
      auto& pass = framebuffer_infos.lookup(i);
      pass.pass = m_render_pass.get();

      framebuffers.append(std::move(pass));
   }

   return framebuffers;
}
