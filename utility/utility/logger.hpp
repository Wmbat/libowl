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

#include <utility/pointer.hpp>

#include <spdlog/spdlog.h>

#include <cstring>
#include <string_view>

namespace util
{
   class logger
   {
   public:
      logger();
      logger(std::string_view name);

      void debug(const std::string& msg);
      void info(const std::string& msg);
      void warning(const std::string& msg);
      void error(const std::string& msg);

      void flush();

      template <typename... Args>
      void info(std::string_view msg, Args&&... args)
      {
         log.info(msg, std::forward<Args>(args)...);
      }
      template <typename... Args>
      void debug(std::string_view msg, Args&&... args)
      {
         log.debug(msg, std::forward<Args>(args)...);
      }
      template <typename... Args>
      void warning(std::string_view msg, Args&&... args)
      {
         log.warn(msg, std::forward<Args>(args)...);
         log.flush();
      }
      template <typename... Args>
      void error(std::string_view msg, Args&&... args)
      {
         log.error(msg, std::forward<Args>(args)...);
         log.flush();
      }

      auto get_logger() -> spdlog::logger&;
      auto get_logger() const -> const spdlog::logger&;

   private:
      spdlog::logger log;
   };

   class logger_wrapper
   {
   public:
      logger_wrapper(util::logger* p_logger = nullptr);

      void debug(const std::string& msg);
      void info(const std::string& msg);
      void warning(const std::string& msg);
      void error(const std::string& msg);

      template <typename... Args>
      void info(std::string_view msg, Args&&... args)
      {
         if (mp_logger)
         {
            mp_logger->info(msg, std::forward<Args>(args)...);
         }
      }
      template <typename... Args>
      void debug(std::string_view msg, Args&&... args)
      {
         if (mp_logger)
         {
            mp_logger->debug(msg, std::forward<Args>(args)...);
         }
      }
      template <typename... Args>
      void warning(std::string_view msg, Args&&... args)
      {
         if (mp_logger)
         {
            mp_logger->warning(msg, std::forward<Args>(args)...);
         }
      }
      template <typename... Args>
      void error(std::string_view msg, Args&&... args)
      {
         if (mp_logger)
         {
            mp_logger->error(msg, std::forward<Args>(args)...);
         }
      }

      auto get() const -> util::logger*; // NOLINT

      [[nodiscard]] auto take() -> util::logger*;

   private:
      util::logger* mp_logger;
   };

} // namespace util
