#pragma once

#include <util/strong_type.hpp>

#include <monads/result.hpp>

#include <system_error>

namespace vulkan
{
   using error_t = util::strong_type<std::error_code, struct error_code_tag>;

   template <class Any>
   using result = monad::result<Any, error_t>;
} // namespace vulkan
