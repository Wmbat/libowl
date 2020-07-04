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

namespace core
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

      spdlog::logger& get_logger();

   private:
      spdlog::logger log;
   };

   inline void log_info(logger* const plogger, std::string_view message,
      const src_location& loc = src_location::current())
   {
      if (plogger)
      {
         plogger->info(
            "[{0}:{1}] " + std::string{message}, SHORT_FILE(loc.file_name()), loc.line());
      }
   }
   inline void log_debug(logger* const plogger, std::string_view message,
      const src_location& loc = src_location::current())
   {
      if (plogger)
      {
         plogger->debug(
            "[{0}:{1}] " + std::string{message}, SHORT_FILE(loc.file_name()), loc.line());
      }
   }
   inline void log_warn(logger* const plogger, std::string_view message,
      const src_location& loc = src_location::current())
   {
      if (plogger)
      {
         plogger->warn(
            "[{0}:{1}] " + std::string{message}, SHORT_FILE(loc.file_name()), loc.line());
      }
   }
   inline void log_error(logger* const plogger, std::string_view message,
      const src_location& loc = src_location::current())
   {
      if (plogger)
      {
         plogger->error(
            "[{0}:{1}] " + std::string{message}, SHORT_FILE(loc.file_name()), loc.line());
      }
   }

   template <typename... args_>
   void log_info(logger* const plogger, std::string_view message, std::tuple<args_...> args,
      const src_location& loc = src_location::current())
   {
      constexpr size_t n = sizeof...(args_);

      const std::string buff =
         "[{" + std::to_string(n) + "}:{" + std::to_string(n + 1) + "}] " + std::string{message};

      if (plogger)
      {
         std::apply(
            [&](auto&&... data) {
               plogger->info(buff, data..., SHORT_FILE(loc.file_name()), loc.line());
            },
            args);
      }
   }
   template <typename... args_>
   void log_debug(logger* const plogger, std::string_view message, std::tuple<args_...> args,
      const src_location& loc = src_location::current())
   {
      constexpr size_t n = sizeof...(args_);

      const std::string buff =
         "[{" + std::to_string(n) + "}:{" + std::to_string(n + 1) + "}] " + std::string{message};

      if (plogger)
      {
         std::apply(
            [&](auto&&... data) {
               plogger->debug(buff, data..., SHORT_FILE(loc.file_name()), loc.line());
            },
            args);
      }
   }
   template <typename... args_>
   void log_warn(logger* const plogger, std::string_view message, std::tuple<args_...> args,
      const src_location& loc = src_location::current())
   {
      constexpr size_t n = sizeof...(args_);

      const std::string buff =
         "[{" + std::to_string(n) + "}:{" + std::to_string(n + 1) + "}] " + std::string{message};

      if (plogger)
      {
         std::apply(
            [&](auto&&... data) {
               plogger->warn(buff, data..., SHORT_FILE(loc.file_name()), loc.line());
            },
            args);
      }
   }
   template <typename... args_>
   void log_error(logger* const plogger, std::string_view message, std::tuple<args_...> args,
      const src_location& loc = src_location::current())
   {
      constexpr size_t n = sizeof...(args_);

      const std::string buff =
         "[{" + std::to_string(n) + "}:{" + std::to_string(n + 1) + "}] " + std::string{message};

      if (plogger)
      {
         std::apply(
            [&](auto&&... data) {
               plogger->error(buff, data..., SHORT_FILE(loc.file_name()), loc.line());
            },
            args);
      }
   }
} // namespace core
