/**
 * Inspired and modified from https://github.com/microsoft/GSL/blob/master/include/gsl/assert
 */

#pragma once

#include <exception>

#if defined(__clang__) || defined(__GNUC__)
#   define UTIL_LIKELY(x) __builtin_expect(!!(x), 1)
#   define UTIL_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define UTIL_LIKELY(x) (!!(x))
#   define UTIL_UNLIKELY(x) (!!(x))
#endif

#define UTIL_CONTRACT_CHECK(type, cond)                                                           \
   (UTIL_LIKELY(cond) ? static_cast<void>(0) : std::terminate())

#if defined(UTIL_ENFORCE_CONTRACTS)
#   define UTIL_EXPECT(cond) UTIL_CONTRACT_CHECK("precondition", cond)
#   define UTIL_ENSURE(cond) UTIL_CONTRACT_CHECK("postcondition", cond)
#else
#   define UTIL_EXPECT(cond)
#   define UTIL_ENSURE(cond)
#endif
