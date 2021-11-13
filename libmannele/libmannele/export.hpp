/**
 * @file libmannele/export.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 22nd of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBMANNELE_EXPORT_HPP_
#define LIBMANNELE_EXPORT_HPP_

// Normally we don't export class templates (but do complete specializations),
// inline functions, and classes with only inline member functions. Exporting
// classes that inherit from non-exported/imported bases (e.g., std::string)
// will end up badly. The only known workarounds are to not inherit or to not
// export. Also, MinGW GCC doesn't like seeing non-exported functions being
// used before their inline definition. The workaround is to reorder code. In
// the end it's all trial and error.

#if defined(LIBMANNELE_STATIC) // Using static.
#   define LIBMANNELE_SYMEXPORT
#elif defined(LIBMANNELE_STATIC_BUILD) // Building static.
#   define LIBMANNELE_SYMEXPORT
#elif defined(LIBMANNELE_SHARED) // Using shared.
#   ifdef _WIN32
#      define LIBMANNELE_SYMEXPORT __declspec(dllimport)
#   else
#      define LIBMANNELE_SYMEXPORT
#   endif
#elif defined(LIBMANNELE_SHARED_BUILD) // Building shared.
#   ifdef _WIN32
#      define LIBMANNELE_SYMEXPORT __declspec(dllexport)
#   else
#      define LIBMANNELE_SYMEXPORT
#   endif
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
#   define LIBMANNELE_SYMEXPORT // Using static or shared.
// #  error define LIBMANNELE_STATIC or LIBMANNELE_SHARED preprocessor macro to signal libmannele
// library type being linked
#endif

#endif // LIBMANNELE_EXPORT_HPP_
