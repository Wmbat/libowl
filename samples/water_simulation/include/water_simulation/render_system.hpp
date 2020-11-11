#pragma once

#include <ui/window.hpp>

#include <util/containers/dynamic_array.hpp>
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

using image_index_t = util::strong_type<std::uint32_t, struct image_index_tag, util::arithmetic>;
using frame_index_t = util::strong_type<std::uint32_t, struct frame_index_tag, util::arithmetic>;

using framebuffer_array = util::small_dynamic_array<vkn::framebuffer, expected_image_count>;
using semaphore_array = util::small_dynamic_array<vkn::semaphore, expected_image_count>;

using vertex_bindings_array = util::dynamic_array<vk::VertexInputBindingDescription>;
using vertex_attributes_array = util::dynamic_array<vk::VertexInputAttributeDescription>;

class render_system
{
public:
   struct config
   {
      util::count32_t swapchain_image_count;
   };

   struct create_info
   {
      std::shared_ptr<util::logger> p_logger;
      ui::window* p_window;
   };

   static auto make(create_info&& info) -> util::result<render_system>;

public:
   auto begin_frame() -> image_index_t;
   void record_draw_calls(const std::function<void(vk::CommandBuffer)>& buffer_calls);
   void end_frame();

   void wait();

   auto device() -> vkn::device&; // NOLINT

   auto vertex_bindings() -> vertex_bindings_array;
   auto vertex_attributes() -> vertex_attributes_array;

   auto lookup_configuration() const -> const config&; // NOLINT

private:
   std::shared_ptr<util::logger> mp_logger;

   ui::window* mp_window;

   vkn::context m_context;
   vkn::device m_device;

   vkn::swapchain m_swapchain;
   vkn::render_pass m_swapchain_render_pass;
   framebuffer_array m_swapchain_framebuffers;

   semaphore_array m_render_finished_semaphores;

   std::array<vkn::command_pool, max_frames_in_flight> m_render_command_pools;
   std::array<vkn::semaphore, max_frames_in_flight> m_image_available_semaphores;
   std::array<vkn::fence, max_frames_in_flight> m_in_flight_fences;

   util::dynamic_array<vkn::fence_observer> m_images_in_flight{};

   image_index_t m_current_image_index;
   frame_index_t m_current_frame_index;

   config m_configuration;
};
