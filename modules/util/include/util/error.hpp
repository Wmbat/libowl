#pragma once

#include <util/strong_type.hpp>

#include <monads/result.hpp>

#include <system_error>

namespace util
{
   using error_t = strong_type<std::error_code, struct error_code_tag>;

   template <typename Any>
   using result = monad::result<Any, error_t>;
} // namespace util
