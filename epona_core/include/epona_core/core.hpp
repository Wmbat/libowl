#pragma once

#include "epona_core/details/logger.hpp"
#include "epona_core/vk/detail/includes.hpp"

#if defined(_WIN32) || defined(WIN32)
#   if defined(EPONA_CORE_STATIC)
#      define CORE_API __declspec(dllimport)
#   else
#      define CORE_API __declspec(dllexport)
#   endif
#else
#   define CORE_API
#endif

namespace core
{
   inline void initialize(logger* p_logger)
   {
      if (auto result = volkInitialize(); result != VK_SUCCESS)
      {
         LOG_ERROR(p_logger, "Failed to initialize volk");

         abort();
         // error
      }

      LOG_INFO(p_logger, "volk initialized");

      if (auto result = glfwInit(); result != GLFW_TRUE)
      {
         LOG_ERROR(p_logger, "Failed to initialize glfw");

         abort();
         // error
      }

      LOG_INFO(p_logger, "glfw initialized");
   }
}; // namespace core
