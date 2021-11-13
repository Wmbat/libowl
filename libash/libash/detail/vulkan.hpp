#ifndef LIBASH_DETAIL_VULKAN_HPP_
#define LIBASH_DETAIL_VULKAN_HPP_

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-copy"
#   pragma clang diagnostic ignored "-Wdefaulted-function-deleted"
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <vulkan/vulkan.hpp>

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic pop
#endif

#endif // LIBASH_DETAIL_VULKAN_HPP_
