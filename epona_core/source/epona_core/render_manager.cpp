/**
 * @file render_manager.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#include "epona_core/render_manager.hpp"
#include "epona_core/details/logger.hpp"

namespace core
{
   render_manager::render_manager(core::logger* p_logger) : p_logger(p_logger), vk_runtime(p_logger)
   {
      if (!IS_GLFW_INIT)
      {
         if (auto res = glfwInit(); res != GLFW_TRUE)
         {
            LOG_ERROR(p_logger, "Failed to initialize GLFW");
         }
         else
         {
            LOG_INFO(p_logger, "GLFW initialized");

            IS_GLFW_INIT = true;
         }
      }
   }

   auto render_manager::setup_runtime() -> void { vk_runtime.create_instance(""); }
} // namespace core
