/**
 * @file libcacao/vulkan.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_VULKAN_HPP_
#define LIBCACAO_VULKAN_HPP_

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#if defined(__GNUC__) || defined(__clang__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <vulkan/vulkan.hpp>

#if defined(__GNUC__) || defined(__clang__)
#   pragma GCC diagnostic pop
#endif

#endif // LIBCACAO_VULKAN_HPP_
