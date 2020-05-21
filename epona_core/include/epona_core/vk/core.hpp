/**
 * MIT License
 *
 * Copyright (c) 2020 Wmbat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <volk.h>

#include <GLFW/glfw3.h>

#include <string>

namespace core
{
#if defined( NDEBUG )
   static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
   static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

   inline static std::string result_to_string( VkResult result )
   {
      switch ( result )
      {
         case VK_SUCCESS:
            return "SUCCESS";
            break;
         case VK_NOT_READY:
            return "NOT_READY";
            break;
         case VK_TIMEOUT:
            return "TIMEOUT";
            break;
         case VK_EVENT_SET:
            return "EVENT_SET";
            break;
         case VK_EVENT_RESET:
            return "EVENT_RESET";
            break;
         case VK_INCOMPLETE:
            return "INCOMPLETE";
            break;
         case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "OUT_OF_HOST_MEMORY";
            break;
         case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "OUT_OF_DEVICE_MEMORY";
            break;
         case VK_ERROR_INITIALIZATION_FAILED:
            return "INITIALIZATION_FAILED";
            break;
         case VK_ERROR_DEVICE_LOST:
            return "DEVICE_LOST";
            break;
         case VK_ERROR_MEMORY_MAP_FAILED:
            return "MEMORY_MAP_FAILED";
            break;
         case VK_ERROR_LAYER_NOT_PRESENT:
            return "LAYER_NOT_PRESENT";
            break;
         case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "EXTENSION_NOT_PRESENT";
            break;
         case VK_ERROR_FEATURE_NOT_PRESENT:
            return "FEATURE_NOT_PRESENT";
            break;
         case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "INCOMPATIBLE_DRIVER";
            break;
         case VK_ERROR_TOO_MANY_OBJECTS:
            return "TOO_MANY_OBJECTS";
            break;
         case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "FORMAT_NOT_SUPPORTED";
            break;
         case VK_ERROR_FRAGMENTED_POOL:
            return "FRAGMENTED_POOL";
            break;
         case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "OUT_OF_POOL_MEMORY";
            break;
         case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return "INVALID_EXTERNAL_HANDLE";
            break;
         case VK_ERROR_SURFACE_LOST_KHR:
            return "SURFACE_LOST";
            break;
         case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "NATIVE_WINDOW_IN_USE";
            break;
         case VK_SUBOPTIMAL_KHR:
            return "SUBOPTIMAL";
            break;
         case VK_ERROR_OUT_OF_DATE_KHR:
            return "OUT_OF_DATE";
            break;
         case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "INCOMPATIBLE_DISPLAY";
            break;
         case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VALIDATION_FAILED";
            break;
         case VK_ERROR_INVALID_SHADER_NV:
            return "INVALID_SHADER";
            break;
         case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT";
            break;
         case VK_ERROR_FRAGMENTATION_EXT:
            return "FRAGMENTATION";
            break;
         case VK_ERROR_NOT_PERMITTED_EXT:
            return "NOT_PERMITTED";
            break;
         case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return "FULL_SCREEN_EXCLUSIVE_MODE_LOST";
            break;
         default:
            return "UNKNOWN_RESULT";
            break;
      }
   }
} // namespace EGL
