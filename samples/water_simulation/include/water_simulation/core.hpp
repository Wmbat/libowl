#pragma once

#include <util/error.hpp>

#include <monads/result.hpp>

template <typename Any>
using result = monad::result<Any, util::error_t>;
