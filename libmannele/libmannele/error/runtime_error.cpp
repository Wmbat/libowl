#include <libmannele/error/runtime_error.hpp>

namespace mannele
{
   runtime_error::runtime_error(std::error_condition e) : std::runtime_error(e.message()), m_err(e)
   {}
   runtime_error::runtime_error(std::error_condition e, const std::string& what) :
      std::runtime_error(what), m_err(e)
   {}
   runtime_error::runtime_error(int value, const std::error_category& category) :
      std::runtime_error(category.message(value)), m_err({value, category})
   {}
   runtime_error::runtime_error(int value, const std::error_category& category,
                                const std::string& what) :
      std::runtime_error(what),
      m_err({value, category})
   {}

   auto runtime_error::condition() const noexcept -> const std::error_condition& { return m_err; }
   auto runtime_error::what() const noexcept -> const char* { return std::runtime_error::what(); }

} // namespace mannele
