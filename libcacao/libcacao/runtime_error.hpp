#ifndef LIBCACAO_RUNTIME_ERROR_HPP
#define LIBCACAO_RUNTIME_ERROR_HPP

#include <libcacao/error.hpp>

namespace cacao
{
   class LIBCACAO_SYMEXPORT runtime_error : public std::runtime_error
   {
   public:
      runtime_error(std::error_condition e);
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

} // namespace cacao

#endif // LIBCACAO_RUNTIME_ERROR_HPP

