#include <util/error.hpp>

namespace vml
{
   runtime_error::runtime_error(error_t e) :
      std::runtime_error{e.value().message()}, m_error_condition{e}
   {}
   runtime_error::runtime_error(error_t e, const std::string& what_arg) :
      std::runtime_error{what_arg}, m_error_condition{e}
   {}
   runtime_error::runtime_error(error_t e, const std::string_view what_arg) :
      std::runtime_error{what_arg.data()}, m_error_condition{e}
   {}
   runtime_error::runtime_error(int value, const std::error_category& category) :
      std::runtime_error{category.message(value)}, m_error_condition{{value, category}}
   {}
   runtime_error::runtime_error(int value, const std::error_category& category,
                                const std::string& what_arg) :
      std::runtime_error{what_arg},
      m_error_condition{{value, category}}
   {}
   runtime_error::runtime_error(int value, const std::error_category& category,
                                const std::string_view what_arg) :
      std::runtime_error{what_arg.data()},
      m_error_condition{{value, category}}
   {}

   auto runtime_error::condition() const noexcept -> const error_t& { return m_error_condition; }

   auto runtime_error::what() const noexcept -> const char* { return std::runtime_error::what(); }
} // namespace vml
