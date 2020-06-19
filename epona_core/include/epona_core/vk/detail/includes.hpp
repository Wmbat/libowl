/**
 * @file includes.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, June 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/detail/concepts.hpp"
#include "epona_core/detail/monad/either.hpp"

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include <string>

namespace core::vk::detail
{
#if defined(NDEBUG)
   static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
   static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif
} // namespace core::vk::detail
