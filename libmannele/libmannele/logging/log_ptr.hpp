/**
 * @file libmannele/logging/log_ptr.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Wednesday, 1st of September 2021
 * @brief 
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBMANNELE_LOGGING_LOG_PTR_HPP
#define LIBMANNELE_LOGGING_LOG_PTR_HPP

#include <libmannele/logging/logger.hpp>

namespace mannele
{
   /**
    * @brief lightweight pointer to a mannele::logger
    */
   class log_ptr
   {
   public:
      log_ptr(logger* p_logger = nullptr);

      template <typename... Args>
      void info(fmt::format_string<Args...> msg, Args&&... args)
      {
         if (mp_logger)
         {
            mp_logger->info(msg, std::forward<Args>(args)...);
         }
      }
      template <typename... Args>
      void debug(fmt::format_string<Args...> msg, Args&&... args)
      {
         if (mp_logger)
         {
            mp_logger->debug(msg, std::forward<Args>(args)...);
         }
      }
      template <typename... Args>
      void warning(fmt::format_string<Args...> msg, Args&&... args)
      {
         if (mp_logger)
         {
            mp_logger->warning(msg, std::forward<Args>(args)...);
         }
      }
      template <typename... Args>
      void error(fmt::format_string<Args...> msg, Args&&... args)
      {
         if (mp_logger)
         {
            mp_logger->error(msg, std::forward<Args>(args)...);
         }
      }

      auto get() const -> logger*; // NOLINT

      [[nodiscard]] auto take() -> logger*;

   private:
      logger* mp_logger;
   };
} // namespace mannele 

#endif // LIBMANNELE_LOGGING_LOG_PTR_HPP
