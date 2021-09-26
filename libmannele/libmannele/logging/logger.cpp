/**
 * @file libmannele/logging/logger.cpp
 * @author wmbat wmbat@protonmail.com
 * @date Wednesday, 1st of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#include <libmannele/logging/logger.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

namespace mannele
{
   logger::logger() : log("Default logger")
   {
      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#if defined(LIBMANNELE_ENABLE_DEBUG_LOGGING)
      console_sink->set_level(spdlog::level::trace);
#else //
      console_sink->set_level(spdlog::level::info);
#endif
      console_sink->set_pattern("[%n] [%^%l%$] %v");

      auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.txt", true);
      file_sink->set_level(spdlog::level::trace);
      file_sink->set_pattern("[%H:%M:%S.%f] [%n] [%^%l%$] %v");

      log = spdlog::logger("Default logger", {console_sink, file_sink});
      log.set_level(spdlog::level::trace);
   }

   logger::logger(std::string_view name) : log("")
   {
      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#if defined(LIBMANNELE_ENABLE_DEBUG_LOGGING)
      console_sink->set_level(spdlog::level::trace);
#else //
      console_sink->set_level(spdlog::level::info);
#endif
      console_sink->set_pattern("[%n] [%^%l%$] %v");

      auto file_sink =
         std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{name} + ".logs", true);
      file_sink->set_pattern("[%H:%M:%S.%f] [%n] [%^%l%$] %v");
      file_sink->set_level(spdlog::level::trace);

      log = spdlog::logger(std::string{name}, {console_sink, file_sink});
      log.set_level(spdlog::level::trace);
   }

   void logger::debug(const std::string& msg) { log.debug(msg); }
   void logger::info(const std::string& msg) { log.info(msg); }
   void logger::warning(const std::string& msg) { log.warn(msg); }
   void logger::error(const std::string& msg) { log.error(msg); }

   void logger::flush() { log.flush(); }

   auto logger::get_logger() -> spdlog::logger& { return log; }
   auto logger::get_logger() const -> const spdlog::logger& { return log; }
} // namespace mannele
