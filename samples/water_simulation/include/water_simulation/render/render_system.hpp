#pragma once

#include <water_simulation/render/image.hpp>
#include <water_simulation/render/render_pass.hpp>

#include <ui/window.hpp>

#include <gfx/memory/index_buffer.hpp>
#include <gfx/memory/vertex_buffer.hpp>

#include <util/logger.hpp>
#include <util/strong_type.hpp>

#include <vkn/command_pool.hpp>
#include <vkn/framebuffer.hpp>
#include <vkn/render_pass.hpp>
#include <vkn/swapchain.hpp>
#include <vkn/sync/fence.hpp>
#include <vkn/sync/semaphore.hpp>

static constexpr std::size_t max_frames_in_flight = 2;
static constexpr std::size_t expected_image_count = 3;

using frame_index_t = util::strong_type<std::uint32_t, struct frame_index_tag, util::arithmetic>;

using framebuffer_array = crl::small_dynamic_array<vkn::framebuffer, expected_image_count>;
using semaphore_array = crl::small_dynamic_array<vkn::semaphore, expected_image_count>;

using vertex_bindings_array = crl::dynamic_array<vk::VertexInputBindingDescription>;
using vertex_attributes_array = crl::dynamic_array<vk::VertexInputAttributeDescription>;

class render_system
{
public:
   struct config
   {
      util::count32_t swapchain_image_count;
   };

   struct create_info
   {
      util::logger_wrapper logger;
      vml::non_null<ui::window*> p_window;
   };

   static auto make(create_info&& info) -> util::result<render_system>;

public:
   auto begin_frame() -> image_index_t;
   void render(std::span<render_pass> passes);
   void end_frame();

   void wait();

   auto device() const -> const vkn::device&;       // NOLINT
   auto device() -> vkn::device&;                   // NOLINT
   auto swapchain() const -> const vkn::swapchain&; // NOLINT
   auto swapchain() -> vkn::swapchain&;             // NOLINT

   auto get_depth_attachment() const -> vk::ImageView; // NOLINT

   auto vertex_bindings() -> vertex_bindings_array;
   auto vertex_attributes() -> vertex_attributes_array;

   auto viewport() const -> vk::Viewport; // NOLINT
   auto scissor() const -> vk::Rect2D;    // NOLINT

   [[nodiscard]] auto create_vertex_buffer(const crl::dynamic_array<gfx::vertex>& vertices) const
      -> util::result<gfx::vertex_buffer>;
   [[nodiscard]] auto create_index_buffer(const crl::dynamic_array<std::uint32_t>& indices) const
      -> util::result<gfx::index_buffer>;

   auto lookup_configuration() const -> const config&; // NOLINT

private:
   util::logger_wrapper m_logger;

   ui::window* mp_window;

   vkn::context m_context;
   vkn::device m_device;
   vkn::swapchain m_swapchain;

   image<image_flags::depth_stencil> m_depth_image;

   semaphore_array m_render_finished_semaphores;

   std::array<vkn::command_pool, max_frames_in_flight> m_render_command_pools;
   std::array<vkn::semaphore, max_frames_in_flight> m_image_available_semaphores;
   std::array<vkn::fence, max_frames_in_flight> m_in_flight_fences;

   crl::dynamic_array<vkn::fence_observer> m_images_in_flight{};

   image_index_t m_current_image_index;
   frame_index_t m_current_frame_index;

   config m_configuration;
};
