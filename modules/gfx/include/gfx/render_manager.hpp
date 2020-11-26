#pragma once

#include <functional>
#include <gfx/data_types.hpp>
#include <gfx/memory/camera_buffer.hpp>
#include <gfx/memory/index_buffer.hpp>
#include <gfx/memory/vertex_buffer.hpp>
#include <gfx/render_pass.hpp>
#include <gfx/window.hpp>

#include <ui/window.hpp>

#include <vkn/command_pool.hpp>
#include <vkn/context.hpp>
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
      render_manager(const ui::window& wnd, util::logger_wrapper logger);

      auto subscribe_renderable(const std::string& name, const renderable_data& r) -> bool;
      void update_model_matrix(const std::string& name, const glm::mat4& model);

      void bake(const vkn::shader& vert_shader, const vkn::shader& frag_shader);

      auto get_pipeline() -> vkn::graphics_pipeline&;

      auto start_frame() -> util::index_t;
      void render_frame(const std::function<void(vk::CommandBuffer)> buffer_calls);

      /**
       * Wait for all resources to stop being used
       */
      void wait();

      auto device() -> vkn::device&;

      auto vertex_bindings() -> vkn::vertex_bindings_array;
      auto vertex_attributes() -> vkn::vertex_attributes_array;

   private:
      auto add_pass(const std::string& name, vkn::queue_type queue_type) -> render_pass&;
      void update_camera(uint32_t image_index);

      auto create_swapchain() noexcept -> vkn::swapchain;
      auto create_swapchain_render_pass() noexcept -> vkn::render_pass;
      auto create_swapchain_framebuffers() noexcept -> framebuffer_array;

      auto create_camera_descriptor_pool() noexcept -> vkn::descriptor_pool;
      auto create_camera_buffers() noexcept -> util::dynamic_array<gfx::camera_buffer>;

      auto create_command_pool() noexcept -> std::array<vkn::command_pool, max_frames_in_flight>;
      auto create_render_finished_semaphores() noexcept
         -> util::small_dynamic_array<vkn::semaphore, vkn::expected_image_count.value()>;
      auto create_image_available_semaphores() noexcept
         -> std::array<vkn::semaphore, max_frames_in_flight>;
      auto create_in_flight_fences() noexcept -> std::array<vkn::fence, max_frames_in_flight>;

   private:
      struct renderable
      {
         std::string name;
         vertex_buffer vertex_buffer;
         index_buffer index_buffer;
      };

      util::logger_wrapper m_logger;

      const ui::window& m_wnd;

      vkn::context m_ctx;
      vkn::device m_device;
      vkn::swapchain m_swapchain;
      vkn::render_pass m_swapchain_render_pass;

      framebuffer_array m_swapchain_framebuffers;

      vkn::graphics_pipeline m_graphics_pipeline;

      vkn::descriptor_pool m_camera_descriptor_pool; // Should be recreated with swapchain

      util::small_dynamic_array<vkn::semaphore, vkn::expected_image_count.value()>
         m_render_finished_semaphores;

      std::array<vkn::command_pool, max_frames_in_flight> m_gfx_command_pools;
      std::array<vkn::semaphore, max_frames_in_flight> m_image_available_semaphores;
      std::array<vkn::fence, max_frames_in_flight> m_in_flight_fences;

      util::dynamic_array<vkn::fence_observer> m_images_in_flight{};

      util::dynamic_array<render_pass> m_render_passes;
      std::unordered_map<std::string, util::index_t> m_render_pass_to_index;

      std::size_t m_current_frame{0};

      std::unordered_map<std::string, std::uint32_t> m_renderables_to_index;
      util::dynamic_array<renderable> m_renderables;
      util::dynamic_array<glm::mat4> m_renderable_model_matrices;

      util::dynamic_array<gfx::camera_buffer> m_camera_buffers;
   };
} // namespace gfx
