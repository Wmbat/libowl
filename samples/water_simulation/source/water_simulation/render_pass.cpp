#include <water_simulation/render_pass.hpp>

#include <monads/try.hpp>

#include <range/v3/view/iota.hpp>

namespace vi = ranges::views;

struct render_pass_data
{
   render_pass::create_info& info;

   vk::UniqueRenderPass render_pass{nullptr};

   util::dynamic_array<framebuffer> framebuffers{};
};

auto create_render_pass(render_pass_data&& data) -> result<render_pass_data>
{
   auto& info = data.info;

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

   const util::small_dynamic_array<vk::SubpassDescription, 1> descriptions{
      {.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
       .colorAttachmentCount = 1,
       .pColorAttachments = &colour_ref,
       .pDepthStencilAttachment = &depth_stencil_ref}};

   const util::small_dynamic_array<vk::SubpassDependency, 1> dependencies{
      {.srcSubpass = VK_SUBPASS_EXTERNAL,
       .dstSubpass = 0,
       .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
          vk::PipelineStageFlagBits::eEarlyFragmentTests,
       .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
          vk::PipelineStageFlagBits::eEarlyFragmentTests,
       .srcAccessMask = {},
       .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
          vk::AccessFlagBits::eDepthStencilAttachmentWrite}};

   vk::RenderPassCreateInfo create_info{
      .attachmentCount = static_cast<std::uint32_t>(std::size(attachment_descriptions)),
      .pAttachments = std::data(attachment_descriptions),
      .subpassCount = static_cast<std::uint32_t>(std::size(descriptions)),
      .pSubpasses = std::data(descriptions),
      .dependencyCount = static_cast<std::uint32_t>(std::size(dependencies)),
      .pDependencies = std::data(dependencies)};

   return monad::try_wrap<vk::SystemError>([&] {
             return info.device.createRenderPassUnique(create_info);
          })
      .map_error([](auto&& /*err*/) {
         return to_err_code(render_pass_error::failed_to_create_render_pass);
      })
      .map([&](vk::UniqueRenderPass&& pass) {
         data.render_pass = std::move(pass);

         return std::move(data);
      });
}

auto create_framebuffers(render_pass_data&& data) -> result<render_pass_data>
{
   auto& info = data.info;

   const std::size_t framebuffer_count = std::size(info.framebuffer_create_infos);

   util::dynamic_array<framebuffer> framebuffers;
   framebuffers.reserve(framebuffer_count);

   for (auto i : vi::iota(0u, framebuffer_count))
   {
      auto fb_res = framebuffer::make(std::move(info.framebuffer_create_infos[i]));

      if (auto err = fb_res.error())
      {
         return monad::err(err.value());
      }

      framebuffers.emplace_back(std::move(fb_res).value().value());
   }

   data.framebuffers = std::move(framebuffers);

   return std::move(data);
}

auto render_pass::make(create_info&& info) -> result<render_pass>
{
   return create_render_pass(render_pass_data{.info = info})
      .and_then(create_framebuffers)
      .map([](render_pass_data&& data) {
         render_pass pass;
         pass.m_render_pass = std::move(data.render_pass);
         pass.m_framebuffers = std::move(data.framebuffers);

         return pass;
      });
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
   cmd_buffer.beginRenderPass(
      {.pNext = nullptr,
       .renderPass = m_render_pass.get(),
       .framebuffer = m_framebuffers[framebuffer_index.value()].value(),
       .renderArea = render_area,
       .clearValueCount = static_cast<std::uint32_t>(std::size(clear_colours)),
       .pClearValues = std::data(clear_colours)},
      vk::SubpassContents::eInline);

   std::invoke(m_buff_calls, cmd_buffer);

   cmd_buffer.endRenderPass();
}

struct render_pass_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "render_pass"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return to_string(static_cast<render_pass_error>(err));
   }
};

static const render_pass_error_category render_pass_error_cat{};

auto to_string(render_pass_error err) -> std::string
{
   if (err == render_pass_error::failed_to_create_render_pass)
   {
      return "failed_to_create_render_pass";
   }

   return "UNKNOWN";
}
auto to_err_code(render_pass_error err) -> util::error_t
{
   return {{static_cast<int>(err), render_pass_error_cat}};
};
