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

#ifdef _MSC_VER
#   define VML_ASSUME(cond) __assume(cond)
#elif defined(__GNUC__)
#   define VML_ASSUME(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())
#else
#   define VML_ASSUME(cond) static_cast<void>((cond) ? 0 : 0)
#endif

#define VML_CONTRACT_CHECK(type, cond) (VML_LIKELY(cond) ? static_cast<void>(0) : std::terminate())

#define EXPECT(cond) VML_CONTRACT_CHECK("precondition", cond)
#define ENSURE(cond) VML_CONTRACT_CHECK("postcondition", cond)
