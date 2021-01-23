#pragma once

#include <fluid_simulation/render/render_pass.hpp>

#include <cacao/context.hpp>
#include <cacao/gfx/image.hpp>
#include <cacao/gfx/index_buffer.hpp>
#include <cacao/gfx/vertex_buffer.hpp>
#include <cacao/ui/window.hpp>
#include <cacao/util/strong_type.hpp>
#include <cacao/vulkan/command_pool.hpp>
#include <cacao/vulkan/swapchain.hpp>

static constexpr std::size_t max_frames_in_flight = 2;
static constexpr std::size_t expected_image_count = 3;

using frame_index_t = cacao::strong_type<std::uint32_t, struct frame_index_tag, cacao::arithmetic>;

using framebuffer_array = crl::small_dynamic_array<framebuffer, expected_image_count>;
using semaphore_array = crl::small_dynamic_array<vk::UniqueSemaphore, expected_image_count>;

using vertex_bindings_array = crl::dynamic_array<vk::VertexInputBindingDescription>;
using vertex_attributes_array = crl::dynamic_array<vk::VertexInputAttributeDescription>;

class render_system
{
public:
   struct config
   {
      cacao::count32_t swapchain_image_count;
   };

   struct create_info
   {
      util::logger_wrapper logger;
      util::non_null<ui::window*> p_window;
   };

   static auto make(create_info&& info) -> util::result<render_system>;

public:
   auto begin_frame() -> image_index_t;
   void render(std::span<render_pass> passes);
   void end_frame();

   void wait();

   auto device() const -> const cacao::device&;     // NOLINT
   auto device() -> cacao::device&;                 // NOLINT
   auto swapchain() const -> const vkn::swapchain&; // NOLINT
   auto swapchain() -> vkn::swapchain&;             // NOLINT

   auto get_depth_attachment() const -> vk::ImageView; // NOLINT

   auto vertex_bindings() -> vertex_bindings_array;
   auto vertex_attributes() -> vertex_attributes_array;

   auto viewport() const -> vk::Viewport; // NOLINT
   auto scissor() const -> vk::Rect2D;    // NOLINT

   [[nodiscard]] auto create_vertex_buffer(const crl::dynamic_array<cacao::vertex>& vertices) const
      -> util::result<cacao::vertex_buffer>;
   [[nodiscard]] auto create_index_buffer(const crl::dynamic_array<std::uint32_t>& indices) const
      -> util::result<cacao::index_buffer>;

   auto lookup_configuration() const -> const config&; // NOLINT

private:
   util::logger_wrapper m_logger;

   ui::window* mp_window;

   cacao::context m_context;
   cacao::device m_device;

   vk::UniqueSurfaceKHR m_surface;

   vkn::swapchain m_swapchain;

   cacao::image m_depth_image;

   semaphore_array m_render_finished_semaphores;

   std::array<vkn::command_pool, max_frames_in_flight> m_render_command_pools;
   std::array<vk::UniqueSemaphore, max_frames_in_flight> m_image_available_semaphores;
   std::array<vk::UniqueFence, max_frames_in_flight> m_in_flight_fences;

   crl::dynamic_array<vk::Fence> m_images_in_flight{};

   image_index_t m_current_image_index;
   frame_index_t m_current_frame_index;

   config m_configuration;
};
