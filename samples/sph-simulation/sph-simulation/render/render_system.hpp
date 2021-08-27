#pragma once

#include <sph-simulation/render/core/image.hpp>
#include <sph-simulation/render/core/render_pass.hpp>

#include <libcacao/command_pool.hpp>
#include <libcacao/context.hpp>
#include <libcacao/index_buffer.hpp>
#include <libcacao/swapchain.hpp>
#include <libcacao/vertex_buffer.hpp>
#include <libcacao/window.hpp>

static constexpr std::size_t max_frames_in_flight = 2;
static constexpr std::size_t expected_image_count = 3;

using framebuffer_array = std::vector<framebuffer>;

using vertex_bindings_array = std::vector<vk::VertexInputBindingDescription>;
using vertex_attributes_array = std::vector<vk::VertexInputAttributeDescription>;

class render_system
{
public:
   render_system() = default;
   render_system(util::non_null<cacao::window*> p_window, util::log_ptr logger);

   auto begin_frame() -> mannele::u32;
   void render(std::span<render_pass> passes);
   void end_frame();

   void wait();

   auto device() const -> const cacao::device&;       // NOLINT
   auto device() -> cacao::device&;                   // NOLINT
   auto swapchain() const -> const cacao::swapchain&; // NOLINT
   auto swapchain() -> cacao::swapchain&;             // NOLINT

   auto get_depth_attachment() const -> vk::ImageView; // NOLINT

   auto vertex_bindings() -> vertex_bindings_array;
   auto vertex_attributes() -> vertex_attributes_array;

   auto viewport() const -> vk::Viewport; // NOLINT
   auto scissor() const -> vk::Rect2D;    // NOLINT

   [[nodiscard]] auto create_vertex_buffer(std::span<const cacao::vertex> vertices) const
      -> cacao::vertex_buffer;
   [[nodiscard]] auto create_index_buffer(std::span<const mannele::u32> indices) const
      -> cacao::index_buffer;

private:
   util::log_ptr m_logger;

   cacao::window* mp_window{};

   cacao::context m_context;
   cacao::surface m_surface;
   cacao::device m_device;
   cacao::swapchain m_swapchain;

   std::vector<vk::UniqueSemaphore> m_render_finished_semaphores;

   std::array<cacao::command_pool, max_frames_in_flight> m_render_command_pools;
   std::array<vk::UniqueSemaphore, max_frames_in_flight> m_image_available_semaphores;
   std::array<vk::UniqueFence, max_frames_in_flight> m_in_flight_fences;

   std::vector<vk::Fence> m_images_in_flight{};

   image m_depth_image{};

   mannele::u32 m_current_image_index{};
   mannele::u32 m_current_frame_index{};
};
