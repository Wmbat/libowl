/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/details/logger.hpp"
#include "epona_core/gui/window.hpp"
#include "epona_core/vk/runtime.hpp"

namespace core
{
   class render_manager
   {
   public:
      render_manager(logger* p_logger);

      void setup_runtime();

   private:
      logger* p_logger;

      vk::runtime vk_runtime;

      inline static bool IS_GLFW_INIT = false;
   };
} // namespace core
