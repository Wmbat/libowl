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
   inline void initialize(const std::shared_ptr<util::logger>& p_logger)
   {
      if (auto result = glfwInit(); result != GLFW_TRUE)
      {
         log_error(p_logger, "Failed to initialize glfw");

         abort();
      }

      log_info(p_logger, "[core] glfw initialized");

      if (auto result = glslang::InitializeProcess(); !result)
      {
         log_error(p_logger, "Failed to initialize glslang");

         abort();
      }

      log_info(p_logger, "[core] glslang initialized");
   }

   using error_t = util::strong_type<std::error_code, struct error_code_tag>;

   template <class any_>
   using result = monad::result<any_, error_t>;
}; // namespace core
