/*
 *  Copyright (C) 2018-2019 Wmbat
 *
 *  wmbat@protonmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  You should have received a copy of the GNU General Public License
 *  GNU General Public License for more details.
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <spdlog/spdlog.h>

namespace core
{
   class logger
   {
   public:
      logger();
      logger(std::string_view name);

      void info(std::string const& msg)
      {
         log.info(msg);

#ifndef NDEBUG
         log.flush();
#endif
      }

      void debug(std::string const& msg)
      {
#ifndef NDEBUG
         log.debug(msg);
         log.flush();
#endif
      }

      void warn(std::string const& msg)
      {
         log.warn(msg);
         log.flush();
      }

      void error(std::string const& msg)
      {
         log.error(msg);
         log.flush();
      }

      void flush() { log.flush(); }

      template <typename... _args>
      void info(std::string_view msg, _args const&... args)
      {
         log.info(msg, args...);

#ifndef NDEBUG
         log.flush();
#endif
      }

      template <typename... _args>
      void debug(std::string_view msg, _args const&... args)
      {
#ifndef NDEBUG
         log.debug(msg, args...);
         log.flush();
#endif
      }

      template <typename... _args>
      void warn(std::string_view msg, _args const&... args)
      {
         log.warn(msg, args...);
         log.flush();
      }

      template <typename... _args>
      void error(std::string_view msg, _args const&... args)
      {
         log.error(msg, args...);
         log.flush();
      }

      spdlog::logger& get_logger();

   private:
      spdlog::logger log;
   };

#define LOG_INFO(p_logger, message)                                                                \
   if (p_logger)                                                                                   \
   {                                                                                               \
      std::string buffer = "[{0}] ";                                                               \
      buffer.append(message);                                                                      \
                                                                                                   \
      p_logger->info(buffer, __FUNCTION__);                                                        \
   }

#define LOG_INFO_P(p_logger, message, ...)                                                         \
   if (p_logger)                                                                                   \
   {                                                                                               \
      std::string buffer = "[{0}] ";                                                               \
      buffer.append(message);                                                                      \
                                                                                                   \
      p_logger->info(buffer, __FUNCTION__, __VA_ARGS__);                                           \
   }

#define LOG_DEBUG(p_logger, message)                                                               \
   if (p_logger)                                                                                   \
   {                                                                                               \
      std::string buffer = "[{0}] ";                                                               \
      buffer.append(message);                                                                      \
                                                                                                   \
      p_logger->info(buffer, __FUNCTION__)                                                         \
   }

#define LOG_DEBUG_P(p_logger, message, ...)                                                        \
   if (p_logger)                                                                                   \
   {                                                                                               \
      std::string buffer = "[{0}] ";                                                               \
      buffer.append(message);                                                                      \
                                                                                                   \
      p_logger->debug(buffer, __FUNCTION__, __VA_ARGS__);                                          \
   }

#define LOG_WARN(p_logger, message)                                                                \
   if (p_logger)                                                                                   \
   {                                                                                               \
      std::string buffer = "[{0}] ";                                                               \
      buffer.append(message);                                                                      \
                                                                                                   \
      p_logger->warn(buffer, __FUNCTION__)                                                         \
   }

#define LOG_WARN_P(p_logger, message, ...)                                                         \
   if (p_logger)                                                                                   \
   {                                                                                               \
      std::string buffer = "[{0}] ";                                                               \
      buffer.append(message);                                                                      \
                                                                                                   \
      p_logger->warn(buffer, __FUNCTION__, __VA_ARGS__);                                           \
   }

#define LOG_ERROR(p_logger, message)                                                               \
   if (p_logger)                                                                                   \
   {                                                                                               \
      std::string buffer = "[{0}] ";                                                               \
      buffer.append(message);                                                                      \
                                                                                                   \
      p_logger->error(buffer, __FUNCTION__);                                                       \
   }

#define LOG_ERROR_P(p_logger, message, ...)                                                        \
   if (p_logger)                                                                                   \
   {                                                                                               \
      std::string buffer = "[{0}] ";                                                               \
      buffer.append(message);                                                                      \
                                                                                                   \
      p_logger->error(buffer, __FUNCTION__, __VA_ARGS__);                                          \
   }
} // namespace core
