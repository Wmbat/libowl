/**
 * Inspired and modified from https://github.com/microsoft/GSL/blob/master/include/gsl/assert
 */

#pragma once

#include <exception>

#if defined(__clang__) || defined(__GNUC__)
#   define VML_LIKELY(x) __builtin_expect(!!(x), 1)
#   define VML_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define VML_LIKELY(x) (!!(x))
#   define VML_UNLIKELY(x) (!!(x))
#endif

#define VML_CONTRACT_CHECK(type, cond) (VML_LIKELY(cond) ? static_cast<void>(0) : std::terminate())

#if defined(VML_ENFORCE_CONTRACTS)
#   define EXPECT(cond) VML_CONTRACT_CHECK("precondition", cond)
#   define ENSURE(cond) VML_CONTRACT_CHECK("postcondition", cond)
#else
#   define EXPECT(cond)
#   define ENSURE(cond)
#endif
