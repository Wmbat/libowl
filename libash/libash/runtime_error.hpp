#ifndef LIBASH_RUNTIME_ERROR_HPP_
#define LIBASH_RUNTIME_ERROR_HPP_

// C++ Standard Library

#include <libmannele/core/source_location.hpp>

#include <source_location>

#include <string>
#include <system_error>

namespace ash::inline v0
{
   class runtime_error : public std::runtime_error
   {
      using super = std::runtime_error;

   public:
      explicit runtime_error(std::error_condition e, mannele::source_location location);

      [[nodiscard]] auto condition() const noexcept -> const std::error_condition&;
      [[nodiscard]] auto what() const noexcept -> const char* override;

   private:
      std::error_condition m_err;
   };
} // namespace ash::inline v0

#endif // LIBASH_RUNTIME_ERROR_HPP_
