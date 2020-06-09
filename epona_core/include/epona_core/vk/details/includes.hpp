/**
 * @file includes.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, June 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/details/concepts.hpp"

#include <volk.h>

#include <GLFW/glfw3.h>

#include <string>

namespace core::vk::details
{
#if defined(NDEBUG)
   static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
   static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif
} // namespace core::vk::details
