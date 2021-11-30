#ifndef LIBASH_RUNTIME_ERROR_HPP_
#define LIBASH_RUNTIME_ERROR_HPP_

// C++ Standard Library

#include <string>
#include <system_error>

namespace ash::inline v0
{
   class runtime_error : public std::runtime_error
   {
   public:
      explicit runtime_error(std::error_condition e);
      runtime_error(std::error_condition e, const std::string& what);
      runtime_error(std::error_condition e, const std::string_view what);
      runtime_error(int value, const std::error_category& category);
      runtime_error(int value, const std::error_category& category, const std::string& what);
      runtime_error(int value, const std::error_category& category, const std::string_view what);

      [[nodiscard]] auto condition() const noexcept -> const std::error_condition&;
      [[nodiscard]] auto what() const noexcept -> const char* override;

   private:
      std::error_condition m_err;
   };
} // namespace ash::inline v0

#endif // LIBASH_RUNTIME_ERROR_HPP_
