#include <libash/runtime_error.hpp>

#include <fmt/format.h>

#include <string>

namespace ash::inline v0
{
   runtime_error::runtime_error(std::error_condition e, mannele::source_location location) :
      super(fmt::format("Runtime error at {}:{}: in function {}:\n\tReason for failure: {}", location.file_name(),
                        location.line(), location.function_name(), e.message())),
      m_err(e)
   {}

   auto runtime_error::condition() const noexcept -> const std::error_condition& { return m_err; }
   auto runtime_error::what() const noexcept -> const char* { return super::what(); }
} // namespace ash::inline v0
