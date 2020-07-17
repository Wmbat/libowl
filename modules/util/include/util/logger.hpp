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

#include <cstring>
#include <experimental/source_location>
#include <string_view>

#if !defined(__FILENAME__)
#   define SHORT_FILE(name) (strrchr(name, '/') ? strrchr(name, '/') + 1 : name)
#endif

namespace util
{
   using src_location = std::experimental::source_location;

   class logger
   {
   public:
      logger();
      logger(std::string_view name);

      void info(std::string const& msg)
      {
         log.info(msg);
         log.flush();
      }

      void debug(std::string const& msg)
      {
         log.debug(msg);
         log.flush();
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

      void info(std::string_view msg, const auto&... args)
      {
         log.info(msg, args...);

#ifndef NDEBUG
         log.flush();
#endif
      }

      void debug(std::string_view msg, const auto&... args)
      {
#ifndef NDEBUG
         log.debug(msg, args...);
         log.flush();
#endif
      }

      void warn(std::string_view msg, const auto&... args)
      {
         log.warn(msg, args...);
         log.flush();
      }

      void error(std::string_view msg, const auto&... args)
      {
         log.error(msg, args...);
         log.flush();
      }

      auto get_logger() -> spdlog::logger&;
      auto get_logger() const -> const spdlog::logger&;

   private:
      spdlog::logger log;
   };

   inline void log_info(logger* const plogger, std::string_view message)
   {
      if (plogger)
      {
         plogger->info(message);
      }
   }
   inline void log_debug(logger* const plogger, std::string_view message)
   {
      if (plogger)
      {
         plogger->debug(message);
      }
   }
   inline void log_warn(logger* const plogger, std::string_view message)
   {
      if (plogger)
      {
         plogger->warn(message);
      }
   }
   inline void log_error(logger* const plogger, std::string_view message)
   {
      if (plogger)
      {
         plogger->error(message);
      }
   }

   void log_info(logger* const plogger, std::string_view message, const auto&... args)
   {
      if (plogger)
      {
         plogger->info(message, args...);
      }
   }
   void log_debug(logger* const plogger, std::string_view message, const auto&... args)
   {
      if (plogger)
      {
         plogger->debug(message, args...);
      }
   }
   void log_warn(logger* const plogger, std::string_view message, const auto&... args)
   {
      if (plogger)
      {
         plogger->warn(message, args...);
      }
   }
   void log_error(logger* const plogger, std::string_view message, const auto&... args)
   {
      if (plogger)
      {
         plogger->error(message, args...);
      }
   }
} // namespace util
