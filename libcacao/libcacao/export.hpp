/**
 * @file libcacao/export.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_EXPORT_HPP_
#define LIBCACAO_EXPORT_HPP_

// Normally we don't export class templates (but do complete specializations),
// inline functions, and classes with only inline member functions. Exporting
// classes that inherit from non-exported/imported bases (e.g., std::string)
// will end up badly. The only known workarounds are to not inherit or to not
// export. Also, MinGW GCC doesn't like seeing non-exported functions being
// used before their inline definition. The workaround is to reorder code. In
// the end it's all trial and error.

#if defined(LIBCACAO_STATIC)         // Using static.
#  define LIBCACAO_SYMEXPORT
#elif defined(LIBCACAO_STATIC_BUILD) // Building static.
#  define LIBCACAO_SYMEXPORT
#elif defined(LIBCACAO_SHARED)       // Using shared.
#  ifdef _WIN32
#    define LIBCACAO_SYMEXPORT __declspec(dllimport)
#  else
#    define LIBCACAO_SYMEXPORT
#  endif // _WIN32
#elif defined(LIBCACAO_SHARED_BUILD) // Building shared.
#  ifdef _WIN32
#    define LIBCACAO_SYMEXPORT __declspec(dllexport)
#  else
#    define LIBCACAO_SYMEXPORT
#  endif // _WIN32
#else
// If none of the above macros are defined, then we assume we are being used
// by some third-party build system that cannot/doesn't signal the library
// type. Note that this fallback works for both static and shared libraries
// provided the library only exports functions (in other words, no global
// exported data) and for the shared case the result will be sub-optimal
// compared to having dllimport. If, however, your library does export data,
// then you will probably want to replace the fallback with the (commented
// out) error since it won't work for the shared case.
// 
#  define LIBCACAO_SYMEXPORT         // Using static or shared.
// #  error define LIBCACAO_STATIC or LIBCACAO_SHARED preprocessor macro to signal libcacao library type being linked
#endif

#endif // LIBCACAO_EXPORT_HPP_
