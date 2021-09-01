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

#ifndef LIBMANNELE_LOGGING_LOGGER_HPP
#define LIBMANNELE_LOGGING_LOGGER_HPP

#include <spdlog/spdlog.h>

#include <cstring>
#include <string_view>

namespace mannele 
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
      void info(fmt::format_string<Args...> msg, Args&&... args)
      {
         log.info(msg, std::forward<Args>(args)...);
      }
      template <typename... Args>
      void debug(fmt::format_string<Args...> msg, Args&&... args)
      {
         log.debug(msg, std::forward<Args>(args)...);
      }
      template <typename... Args>
      void warning(fmt::format_string<Args...> msg, Args&&... args)
      {
         log.warn(msg, std::forward<Args>(args)...);
         log.flush();
      }
      template <typename... Args>
      void error(fmt::format_string<Args...> msg, Args&&... args)
      {
         log.error(msg, std::forward<Args>(args)...);
         log.flush();
      }

      auto get_logger() -> spdlog::logger&;
      auto get_logger() const -> const spdlog::logger&;

   private:
      spdlog::logger log;
   };
} // namespace mannele

#endif // LIBMANNELE_LOGGING_LOGGER_HPP
