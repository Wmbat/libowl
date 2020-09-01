#pragma once

#include <util/logger.hpp>
#include <util/strong_type.hpp>

#include <monads/result.hpp>

#include <system_error>

namespace gfx
{
   using error_t = util::strong_type<std::error_code, struct error_code_tag>;

   template <class any_>
   using result = monad::result<any_, error_t>;
} // namespace gfx
