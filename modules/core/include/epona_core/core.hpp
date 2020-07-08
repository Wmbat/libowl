#pragma once

#include <epona_util/logger.hpp>

#include <GLFW/glfw3.h>

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
   inline void initialize(util::logger* const plogger)
   {
      if (auto result = glfwInit(); result != GLFW_TRUE)
      {
         log_error(plogger, "Failed to initialize glfw");

         abort();
         // error
      }

      log_info(plogger, "glfw initialized");
   }
}; // namespace core
