/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <core/graphics/gui/window.hpp>
#include <core/shader_codex.hpp>

#include <gfx/memory/index_buffer.hpp>
#include <gfx/memory/vertex_buffer.hpp>

#include <util/containers/dense_hash_map.hpp>

#include <vkn/command_pool.hpp>
#include <vkn/core.hpp>
#include <vkn/descriptor_pool.hpp>
#include <vkn/device.hpp>
#include <vkn/framebuffer.hpp>
#include <vkn/instance.hpp>
#include <vkn/pipeline.hpp>
#include <vkn/render_pass.hpp>
#include <vkn/shader.hpp>
#include <vkn/swapchain.hpp>
#include <vkn/sync/fence.hpp>
#include <vkn/sync/semaphore.hpp>

#include <util/logger.hpp>
#include <util/strong_type.hpp>

namespace core
{
   class render_manager
   {
      static constexpr std::size_t max_frames_in_flight = 2;

   public:
      render_manager(gfx::window* const p_wnd,
                     const std::shared_ptr<util::logger>& p_logger = nullptr);

      void render_frame();
      /**
       * Wait for all resources to stop being used
       */
      void wait();

   private:
      void recreate_swapchain();

      auto create_physical_device() const noexcept -> vkn::physical_device;
      auto create_swapchain_framebuffers() const noexcept
         -> util::small_dynamic_array<vkn::framebuffer, vkn::expected_image_count.value()>;

   private:
      gfx::window* const mp_window;

      std::shared_ptr<util::logger> mp_logger;

      std::string m_engine_name = "Epona";

      vkn::loader m_loader;
      vkn::instance m_instance;
      vkn::device m_device;
      vkn::swapchain m_swapchain;
      vkn::render_pass m_render_pass;
      vkn::command_pool m_command_pool;
      vkn::descriptor_pool m_descriptor_pool; // Should be recreated with swapchain
      vkn::graphics_pipeline m_graphics_pipeline;

      util::small_dynamic_array<vkn::framebuffer, vkn::expected_image_count.value()> m_framebuffers;

      std::array<vkn::semaphore, max_frames_in_flight> m_image_available_semaphores;
      std::array<vkn::semaphore, max_frames_in_flight> m_render_finished_semaphores;
      std::array<vkn::fence, max_frames_in_flight> m_in_flight_fences;
      util::dynamic_array<vkn::fence_observer> m_images_in_flight{};

      shader_codex m_shader_codex;

      std::size_t m_current_frame{0};

      ::gfx::vertex_buffer m_vertex_buffer;
      ::gfx::index_buffer m_index_buffer;

      util::dynamic_array<::gfx::vertex> m_triangle_vertices{
         {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
         {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
         {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
         {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}};

      util::dynamic_array<std::uint32_t> m_triangle_indices{0, 1, 2, 2, 3, 0};
   };
} // namespace core
