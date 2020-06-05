/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/details/logger.hpp"
#include "epona_core/gui/window.hpp"
#include "epona_core/vk/details/includes.hpp"
#include "epona_core/vk/details/result.hpp"
#include "epona_core/vk/device.hpp"
#include "epona_core/vk/instance.hpp"
#include "epona_core/vk/runtime.hpp"

namespace core
{
   class render_manager
   {
   public:
      render_manager(window* p_wnd, logger* p_logger = nullptr);

   private:
      window* p_window;
      logger* p_logger;

      std::string engine_name = "Epona";

      vk::runtime runtime;
      vk::instance instance;
      vk::device device;
   };
} // namespace core
