/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/detail/logger.hpp"
#include "epona_core/graphics/vkn/instance.hpp"
#include "epona_core/graphics/vkn/core.hpp"
#include "epona_core/gui/window.hpp"
/*
#include "epona_core/vk/detail/includes.hpp"
#include "epona_core/vk/detail/result.hpp"
#include "epona_core/vk/device.hpp"
#include "epona_core/vk/instance.hpp"
#include "epona_core/vk/runtime.hpp"
*/

namespace core
{
   class render_manager
   {
   public:
      render_manager(window* const p_wnd, logger* const p_logger = nullptr);

   private:
      window* const p_window;
      logger* const p_logger;

      std::string engine_name = "Epona";

      gfx::vkn::loader loader;
      gfx::vkn::instance instance;

      /*
      vk::runtime runtime;
      vk::instance instance;
      vk::device device;
      */
   };
} // namespace core
