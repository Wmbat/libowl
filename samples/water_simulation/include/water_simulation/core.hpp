#pragma once

#include <util/error.hpp>

#include <monads/result.hpp>

#include <filesystem>

template <typename Any>
using result = monad::result<Any, util::error_t>;

using filepath = std::filesystem::path;
