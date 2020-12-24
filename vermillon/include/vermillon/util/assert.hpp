/**
 * Inspired and modified from https://github.com/microsoft/GSL/blob/master/include/gsl/assert
 */

#pragma once

#include <exception>

#if defined(__clang__) || defined(__GNUC__)
#   define CACAO_LIKELY(x) __builtin_expect(!!(x), 1)
#   define CACAO_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define CACAO_LIKELY(x) (!!(x))
#   define CACAO_UNLIKELY(x) (!!(x))
#endif

#define CACAO_CONTRACT_CHECK(type, cond)                                                           \
   (CACAO_LIKELY(cond) ? static_cast<void>(0) : std::terminate())

#if defined(CACAO_ENFORCE_CONTRACTS)
#   define EXPECT(cond) CACAO_CONTRACT_CHECK("precondition", cond)
#   define ENSURE(cond) CACAO_CONTRACT_CHECK("postcondition", cond)
#else
#   define EXPECT(cond)
#   define ENSURE(cond)
#endif
