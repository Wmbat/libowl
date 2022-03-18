#include <libash/runtime_error.hpp>

#include <fmt/format.h>

#include <string>

namespace ash::inline v0
{
   runtime_error::runtime_error(std::error_condition e, std::string_view reason,
                                mannele::source_location location) :
      super(fmt::format("libash runtime error at {}:{}: in function {}:\n\terror code: "
                        "{}\n\treason for failure: {}\n",
                        location.file_name(), location.line(), location.function_name(),
                        e.message(), reason)),
      m_err(e)
   {}

   auto runtime_error::condition() const noexcept -> const std::error_condition& { return m_err; }
   auto runtime_error::what() const noexcept -> const char* { return super::what(); }
} // namespace ash::inline v0
