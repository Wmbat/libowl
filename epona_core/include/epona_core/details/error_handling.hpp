/**
 * @file error_handling.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 3rd, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/details/logger.hpp"

#include <string_view>

namespace core
{
#ifndef ESL_NO_LOGGING

   inline static logger ESL_error_logger{"ESL error logger"};
#endif

   static inline void handle_bad_alloc_error([[maybe_unused]] std::string_view error_msg)
   {
#ifndef ESL_NO_EXCEPTIONS
      throw std::bad_alloc{};
#else
#   ifndef ESL_NO_LOGGING
      ESL_error_logger.error(error_msg);
#   endif

      abort();
#endif
   }

   static inline void handle_out_of_range_error([[maybe_unused]] std::string_view error_msg)
   {
#ifndef ESL_NO_EXCEPTIONS
      throw std::out_of_range{error_msg.data()};
#else
#   ifndef ESL_NO_LOGGING
      ESL_error_logger.error(error_msg);
#   endif

      abort();
#endif
   }
} // namespace ESL
