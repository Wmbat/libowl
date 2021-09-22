/**
 * @file libmannele/logging/logger.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Wednesday, 1st of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBMANNELE_LOGGING_LOGGER_HPP_
#define LIBMANNELE_LOGGING_LOGGER_HPP_

#include <libmannele/export.hpp>

// Third Party Libraries

#include <spdlog/spdlog.h>

// C++ Standard Library

#include <cstring>
#include <string>
#include <string_view>
#include <utility>

namespace mannele
{
   /**
    * @brief A simple logging class built on top of spdlog
    */
   class LIBMANNELE_SYMEXPORT logger
   {
   public:
      logger();
      logger(std::string_view name); // NOLINT

      /**
       * @brief Log a debug message
       *
       * @param[in] msg The message to log
       */
      void debug(const std::string& msg);
      /**
       * @brief Log an info message
       *
       * @param[in] msg The message to log
       */
      void info(const std::string& msg);
      /**
       * @brief Log a warning message
       *
       * @param[in] msg The message to log
       */
      void warning(const std::string& msg);
      /**
       * @brief Log an error message
       *
       * @param[in] msg The message to log
       */
      void error(const std::string& msg);

      /**
       * @brief Flush the logger's buffer
       */
      void flush();

      /**
       * @brief Log a info message with formatting
       *
       * @param[in] msg String containing formatting elements
       * @param[in] args The elements to formats
       */
      template <typename... Args>
      void info(fmt::format_string<Args...> msg, Args&&... args)
      {
         log.info(msg, std::forward<Args>(args)...);
      }
      /**
       * @brief Log a debug message with formatting
       *
       * @param[in] msg String containing formatting elements
       * @param[in] args The elements to formats
       */
      template <typename... Args>
      void debug(fmt::format_string<Args...> msg, Args&&... args)
      {
         log.debug(msg, std::forward<Args>(args)...);
      }
      /**
       * @brief Log a warning message with formatting
       *
       * @param[in] msg String containing formatting elements
       * @param[in] args The elements to formats
       */
      template <typename... Args>
      void warning(fmt::format_string<Args...> msg, Args&&... args)
      {
         log.warn(msg, std::forward<Args>(args)...);
         log.flush();
      }
      /**
       * @brief Log an error message with formatting
       *
       * @param[in] msg String containing formatting elements
       * @param[in] args The elements to formats
       */
      template <typename... Args>
      void error(fmt::format_string<Args...> msg, Args&&... args)
      {
         log.error(msg, std::forward<Args>(args)...);
         log.flush();
      }

      /**
       * @brief Access the underlying spdlog logger
       */
      auto get_logger() -> spdlog::logger&;
      /**
       * @brief Access the underlying spdlog logger
       */
      auto get_logger() const -> const spdlog::logger&;

   private:
      spdlog::logger log;
   };
} // namespace mannele

#endif // LIBMANNELE_LOGGING_LOGGER_HPP_
