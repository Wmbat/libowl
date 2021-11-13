/**
 * @file libmannele/error/runtime_error.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 22nd of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBMANNELE_RUNTIME_ERROR_HPP_
#define LIBMANNELE_RUNTIME_ERROR_HPP_

// C++ Standard Library

#include <stdexcept>
#include <string>
#include <system_error> // NOLINT

namespace mannele
{
   class runtime_error : public std::runtime_error
   {
   public:
      runtime_error(std::error_condition e); // NOLINT
      runtime_error(std::error_condition e, const std::string& what);
      runtime_error(int value, const std::error_category& category);
      runtime_error(int value, const std::error_category& category, const std::string& what);

      [[nodiscard]] auto condition() const noexcept -> const std::error_condition&;
      [[nodiscard]] auto what() const noexcept -> const char* override;

   private:
      std::error_condition m_err;
   };
} // namespace mannele

#endif // LIBMANNELE_RUNTIME_ERROR_HPP_
