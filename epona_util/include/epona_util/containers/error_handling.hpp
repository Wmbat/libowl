/**
 * @file error_handling.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 3rd, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <epona_util/logger.hpp>

#include <string_view>

namespace util
{
#ifndef UTIL_NO_LOGGING

   inline static logger util_error_logger{"epone_util_logger"};
#endif

   static inline void handle_bad_alloc_error([[maybe_unused]] std::string_view error_msg)
   {
#ifndef UTIL_NO_EXCEPTIONS
      throw std::bad_alloc{};
#else
#   ifndef UTIL_NO_LOGGING
      util_error_logger.error(error_msg);
#   endif

      abort();
#endif
   }

   static inline void handle_out_of_range_error([[maybe_unused]] std::string_view error_msg)
   {
#ifndef UTIL_NO_EXCEPTIONS
      throw std::out_of_range{error_msg.data()};
#else
#   ifndef UTIL_NO_LOGGING
      util_error_logger.error(error_msg);
#   endif

      abort();
#endif
   }
} // namespace util
