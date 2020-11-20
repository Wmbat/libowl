#pragma once

#include <water_simulation/core.hpp>
#include <water_simulation/render/framebuffer.hpp>

#include <vkn/core.hpp>

#include <span>

enum struct render_pass_error
{
   failed_to_create_render_pass
};

auto to_string(render_pass_error err) -> std::string;
auto to_err_code(render_pass_error err) -> util::error_t;

class render_pass
{
public:
   struct create_info
   {
      vk::Device device;
      vk::SwapchainKHR swapchain;

      monad::maybe<vk::AttachmentDescription> colour_attachment{monad::none};
      monad::maybe<vk::AttachmentDescription> depth_stencil_attachment{monad::none};

      util::dynamic_array<framebuffer::create_info> framebuffer_create_infos{};

      std::shared_ptr<util::logger> logger{nullptr};
   };

   static auto make(create_info&& info) -> result<render_pass>;

public:
   auto value() const -> vk::RenderPass; // NOLINT

   void record_render_calls(const std::function<void(vk::CommandBuffer)>& calls);

   void submit_render_calls(vk::CommandBuffer cmd_buffer, util::index_t framebuffer_index,
                            vk::Rect2D render_area, std::span<const vk::ClearValue> clear_colours);

private:
   vk::UniqueRenderPass m_render_pass;

   util::dynamic_array<framebuffer> m_framebuffers;

   std::function<void(vk::CommandBuffer)> m_buff_calls;
};
