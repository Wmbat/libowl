#pragma once

#include <gfx/context.hpp>
#include <gfx/data_types.hpp>
#include <gfx/memory/camera_buffer.hpp>
#include <gfx/memory/index_buffer.hpp>
#include <gfx/memory/vertex_buffer.hpp>
#include <gfx/render_pass.hpp>
#include <gfx/window.hpp>

#include <core/shader_codex.hpp>

#include <vkn/command_pool.hpp>
#include <vkn/descriptor_pool.hpp>
#include <vkn/device.hpp>
#include <vkn/framebuffer.hpp>
#include <vkn/pipeline.hpp>
#include <vkn/render_pass.hpp>
#include <vkn/swapchain.hpp>
#include <vkn/sync/fence.hpp>
#include <vkn/sync/semaphore.hpp>

namespace gfx
{
   class render_manager
   {
      static constexpr std::size_t max_frames_in_flight = 2;

      using framebuffer_array =
         util::small_dynamic_array<vkn::framebuffer, vkn::expected_image_count.value()>;

   public:
      render_manager(const context& ctx, const window& wnd, std::shared_ptr<util::logger> p_logger);

      auto add_vertex_buffer(const util::dynamic_array<vertex>& vertices) -> bool;
      auto add_index_buffer(const util::dynamic_array<std::uint32_t>& indices) -> bool;

      void update_camera(uint32_t image_index);

      void bake();

      void render_frame();

      /**
       * Wait for all resources to stop being used
       */
      void wait();

   private:
      auto add_pass(const std::string& name, vkn::queue::type queue_type) -> render_pass&;

      auto create_physical_device() const noexcept -> vkn::physical_device;
      auto create_logical_device() const noexcept -> vkn::device;
      auto create_swapchain() const noexcept -> vkn::swapchain;
      auto create_swapchain_render_pass() const noexcept -> vkn::render_pass;
      auto create_swapchain_framebuffers() const noexcept -> framebuffer_array;
      auto create_command_pool() const noexcept -> vkn::command_pool;
      auto create_shader_codex() const noexcept -> core::shader_codex;

      auto create_camera_descriptor_pool() const noexcept -> vkn::descriptor_pool;
      auto create_camera_buffers() const noexcept -> util::dynamic_array<gfx::camera_buffer>;

      auto create_image_available_semaphores() const noexcept
         -> std::array<vkn::semaphore, max_frames_in_flight>;
      auto create_render_finished_semaphores() const noexcept
         -> std::array<vkn::semaphore, max_frames_in_flight>;
      auto create_in_flight_fences() const noexcept -> std::array<vkn::fence, max_frames_in_flight>;

   private:
      std::shared_ptr<util::logger> mp_logger;

      const context& m_ctx;
      const window& m_wnd;

      vkn::device m_device;
      vkn::swapchain m_swapchain;
      vkn::render_pass m_swapchain_render_pass;
      framebuffer_array m_swapchain_framebuffers;

      vkn::graphics_pipeline m_graphics_pipeline;

      vkn::descriptor_pool m_camera_descriptor_pool; // Should be recreated with swapchain

      vkn::command_pool m_command_pool;

      std::array<vkn::semaphore, max_frames_in_flight> m_image_available_semaphores;
      std::array<vkn::semaphore, max_frames_in_flight> m_render_finished_semaphores;
      std::array<vkn::fence, max_frames_in_flight> m_in_flight_fences;

      util::dynamic_array<vkn::fence_observer> m_images_in_flight{};

      core::shader_codex m_shader_codex;

      util::dynamic_array<render_pass> m_render_passes;
      std::unordered_map<std::string, util::index_t> m_render_pass_to_index;

      std::size_t m_current_frame{0};

      util::dynamic_array<gfx::vertex_buffer> m_vertex_buffers;
      util::dynamic_array<gfx::index_buffer> m_index_buffers;
      util::dynamic_array<gfx::camera_buffer> m_camera_buffers;
   };
} // namespace gfx
