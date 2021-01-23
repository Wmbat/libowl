/*
 *  Copyright (C) 2020 Wmbat
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

#include <utility/logger.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

namespace util
{
   logger::logger() : log("Default logger")
   {
      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      console_sink->set_pattern("[%H:%M:%S.%f] [%n] [%^%l%$] %v");
      console_sink->set_level(spdlog::level::trace);

      auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.txt", true);
      file_sink->set_pattern("[%H:%M:%S.%f] [%n] [%^%l%$] %v");
      file_sink->set_level(spdlog::level::trace);

      log = spdlog::logger("Default logger", {console_sink, file_sink});
      log.set_level(spdlog::level::trace);
   }

   logger::logger(std::string_view name) : log("")
   {
      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      console_sink->set_pattern("[%H:%M:%S.%f] [%n] [%^%l%$] %v");
      console_sink->set_level(spdlog::level::trace);

      auto file_sink =
         std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{name} + ".logs", true);
      file_sink->set_pattern("[%H:%M:%S.%f] [%n] [%^%l%$] %v");
      file_sink->set_level(spdlog::level::trace);

      log = spdlog::logger(std::string{name}, {console_sink, file_sink});
#if defined(CACAO_DEBUG_LOGGING)
      log.set_level(spdlog::level::trace);
#else
      log.set_level(spdlog::level::info);
#endif
   }

   void logger::debug(const std::string& msg) { log.debug(msg); }
   void logger::info(const std::string& msg) { log.info(msg); }
   void logger::warning(const std::string& msg) { log.warn(msg); }
   void logger::error(const std::string& msg) { log.error(msg); }

   void logger::flush() { log.flush(); }

   auto logger::get_logger() -> spdlog::logger& { return log; }
   auto logger::get_logger() const -> const spdlog::logger& { return log; }

   logger_wrapper::logger_wrapper(util::logger* p_logger) : mp_logger{p_logger} {}

   void logger_wrapper::debug(const std::string& msg)
   {
      if (mp_logger)
      {
         mp_logger->debug(msg);
      }
   }
   void logger_wrapper::info(const std::string& msg)
   {
      if (mp_logger)
      {
         mp_logger->info(msg);
      }
   }
   void logger_wrapper::warning(const std::string& msg)
   {
      if (mp_logger)
      {
         mp_logger->warning(msg);
      }
   }
   void logger_wrapper::error(const std::string& msg)
   {
      if (mp_logger)
      {
         mp_logger->error(msg);
      }
   }

   auto logger_wrapper::get() const -> util::logger* { return mp_logger; }
   auto logger_wrapper::take() -> util::logger*
   {
      auto* temp = mp_logger;
      mp_logger = nullptr;

      return temp;
   }
} // namespace util
