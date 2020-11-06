#pragma once

#include <util/strong_type.hpp>

#include <system_error>

namespace util
{
   using error_t = strong_type<std::error_code, struct error_code_tag>;
} // namespace util
