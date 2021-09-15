#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/render/core/framebuffer.hpp>

#include <libcacao/device.hpp>

#include <libmannele/core.hpp>

#include <libreglisse/maybe.hpp>

#include <span>

struct render_pass_create_info
{
   cacao::device& device;

   reglisse::maybe<vk::AttachmentDescription> colour_attachment;
   reglisse::maybe<vk::AttachmentDescription> depth_stencil_attachment;

   std::vector<framebuffer_create_info> framebuffer_create_infos{};

   mannele::log_ptr logger{nullptr};
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
   void submit_render_calls(vk::CommandBuffer cmd_buffer, mannele::u64 framebuffer_index,
                            vk::Rect2D render_area, std::span<const vk::ClearValue> clear_colours);

private:
   auto create_render_pass(const render_pass_create_info& info) -> vk::UniqueRenderPass;
   auto create_framebuffers(const render_pass_create_info& info) -> std::vector<framebuffer>;

private:
   vk::UniqueRenderPass m_render_pass;

   std::vector<framebuffer> m_framebuffers;

   std::function<void(vk::CommandBuffer)> m_buff_calls;
};
