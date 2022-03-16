#ifndef LIBOWL_RUNTIME_ERROR_HPP_
#define LIBOWL_RUNTIME_ERROR_HPP_

#include <libmannele/core/source_location.hpp>

#include <stdexcept>
#include <string_view>
#include <system_error>

namespace owl::inline v0
{
   class runtime_error : public std::runtime_error
   {
      using super = std::runtime_error;

   public:
      explicit runtime_error(std::error_condition e, std::string_view reason,
                             mannele::source_location location);

      [[nodiscard]] auto condition() const noexcept -> const std::error_condition&;
      [[nodiscard]] auto what() const noexcept -> const char* override;

   private:
      std::error_condition m_err;
   };
} // namespace owl::inline v0

#endif // LIBOWL_RUNTIME_ERROR_HPP_
