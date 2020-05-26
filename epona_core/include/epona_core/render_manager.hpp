/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/details/logger.hpp"
#include "epona_core/gui/window.hpp"
#include "epona_core/vk/core.hpp"
#include "epona_core/vk/instance.hpp"
#include "epona_core/vk/result.hpp"
#include "epona_core/vk/runtime.hpp"

namespace core
{
   class render_manager
   {
   public:
      render_manager(logger* p_logger);

   private:
      logger* p_logger;

      vk::runtime vk_runtime;
      vk::instance vk_instance;

      std::string engine_name = "Epona";
   };
} // namespace core
