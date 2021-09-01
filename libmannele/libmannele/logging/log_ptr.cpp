#include <libmannele/logging/log_ptr.hpp>

namespace mannele
{
   log_ptr::log_ptr(logger* p_logger) : mp_logger{p_logger} {}

   auto log_ptr::get() const -> logger* { return mp_logger; }
   auto log_ptr::take() -> logger*
   {
      auto* temp = mp_logger;
      mp_logger = nullptr;

      return temp;
   }
} // namespace mannele
