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

namespace util
{
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

   inline void log_info(const std::shared_ptr<logger>& p_logger, std::string_view message)
   {
      if (p_logger)
      {
         p_logger->info(message);
      }
   }
   inline void log_debug(const std::shared_ptr<logger>& p_logger, std::string_view message)
   {
      if (p_logger)
      {
         p_logger->debug(message);
      }
   }
   inline void log_warn(const std::shared_ptr<logger>& p_logger, std::string_view message)
   {
      if (p_logger)
      {
         p_logger->warn(message);
      }
   }
   inline void log_error(const std::shared_ptr<logger>& p_logger, std::string_view message)
   {
      if (p_logger)
      {
         p_logger->error(message);
      }
   }

   inline void log_info(const std::shared_ptr<logger>& p_logger, const std::string& message,
                        const auto&... args)
   {
      if (p_logger)
      {
         p_logger->info(message, args...);
      }
   }
   inline void log_debug(const std::shared_ptr<logger>& p_logger, const std::string& message,
                         const auto&... args)
   {
      if (p_logger)
      {
         p_logger->debug(message, args...);
      }
   }
   inline void log_warn(const std::shared_ptr<logger>& p_logger, const std::string& message,
                        const auto&... args)
   {
      if (p_logger)
      {
         p_logger->warn(message, args...);
      }
   }
   inline void log_error(const std::shared_ptr<logger>& p_logger, const std::string& message,
                         const auto&... args)
   {
      if (p_logger)
      {
         p_logger->error(message, args...);
      }
   }

   using logger_ptr = std::shared_ptr<util::logger>;
} // namespace util
