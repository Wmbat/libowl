#pragma once

#include <util/logger.hpp>
#include <util/strong_type.hpp>

#include <vkn/core.hpp>

#include <monads/result.hpp>

#include <GLFW/glfw3.h>

#include <system_error>

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

      log_info(plogger, "[core] glfw initialized");
   }

   using error_t = util::strong_type<std::error_code, struct error_code_tag>;

   template <class any_>
   using result = monad::result<any_, error_t>;
}; // namespace core
