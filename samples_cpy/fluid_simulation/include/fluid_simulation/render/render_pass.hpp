#pragma once

#include <fluid_simulation/core.hpp>
#include <fluid_simulation/render/framebuffer.hpp>

#include <cacao/vulkan/core.hpp>

#include <span>

struct render_pass_create_info
{
   vk::Device device;
   vk::SwapchainKHR swapchain;

   monad::maybe<vk::AttachmentDescription> colour_attachment{monad::none};
   monad::maybe<vk::AttachmentDescription> depth_stencil_attachment{monad::none};

   crl::dynamic_array<framebuffer::create_info> framebuffer_create_infos{};

   util::logger_wrapper logger{nullptr};
};

class render_pass
{
public:
   render_pass() = default;
   /**
    * @brief Create a render_pass object from user provided information.
    *
    * @param info The information needed to build the render_pass object.
    */
   render_pass(render_pass_create_info&& info);

   /**
    * @brief Access the underlying vulkan render_pass handle
    *
    * @return The handle to the vulkan render_pass.
    */
   auto value() const -> vk::RenderPass; // NOLINT

   /**
    * @brief
    */
   void record_render_calls(const std::function<void(vk::CommandBuffer)>& calls);

   /**
    * @brief
    */
   void submit_render_calls(vk::CommandBuffer cmd_buffer, cacao::index_t framebuffer_index,
                            vk::Rect2D render_area, std::span<const vk::ClearValue> clear_colours);

private:
   auto create_render_pass(const render_pass_create_info& info) -> vk::UniqueRenderPass;
   auto create_framebuffers(const render_pass_create_info& info) -> crl::dynamic_array<framebuffer>;

private:
   vk::UniqueRenderPass m_render_pass;

   crl::dynamic_array<framebuffer> m_framebuffers;

   std::function<void(vk::CommandBuffer)> m_buff_calls;
};
