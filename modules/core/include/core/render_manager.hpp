/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "core/graphics/gui/window.hpp"
#include "core/shader_codex.hpp"

#include <util/containers/dense_hash_map.hpp>

#include <vkn/command_pool.hpp>
#include <vkn/core.hpp>
#include <vkn/device.hpp>
#include <vkn/framebuffer.hpp>
#include <vkn/instance.hpp>
#include <vkn/pipeline.hpp>
#include <vkn/render_pass.hpp>
#include <vkn/shader.hpp>
#include <vkn/swapchain.hpp>
#include <vkn/sync/fence.hpp>
#include <vkn/sync/fence_observer.hpp>
#include <vkn/sync/semaphore.hpp>

#include <util/logger.hpp>

namespace core
{
   class render_manager
   {
      static constexpr std::size_t max_frames_in_flight = 2;

   public:
      render_manager(gfx::window* const p_wnd, util::logger* p_logger = nullptr);

      void render_frame();
      void wait();

   private:
      gfx::window* const mp_window;
      util::logger* const mp_logger;

      std::string m_engine_name = "Epona";

      vkn::loader m_loader;
      vkn::instance m_instance;
      vkn::device m_device;
      vkn::swapchain m_swapchain;
      vkn::render_pass m_render_pass;
      vkn::command_pool m_command_pool;
      vkn::graphics_pipeline m_graphics_pipeline;

      util::small_dynamic_array<vkn::framebuffer, 3> m_framebuffers;

      std::array<vkn::semaphore, max_frames_in_flight> m_image_available_semaphores;
      std::array<vkn::semaphore, max_frames_in_flight> m_render_finished_semaphores;
      std::array<vkn::fence, max_frames_in_flight> m_in_flight_fences;
      util::dynamic_array<vkn::fence_observer> m_images_in_flight;

      shader_codex m_shader_codex;

      std::size_t m_current_frame{0};
   };
} // namespace core
