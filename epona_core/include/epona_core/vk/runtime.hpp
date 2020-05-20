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

#include <array>
#include <epona_core/vk/core.hpp>
#include <epona_core/vk/extension.hpp>

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/details/logger.hpp"
#include "epona_core/memory/multipool_allocator.hpp"

#include <string>
#include <string_view>

namespace core::vk
{
   class runtime
   {
   public:
      runtime(logger* p_logger);

      void create_instance(std::string_view app_name);

   public:
      logger* p_logger;

      std::uint32_t version;

      VkInstance instance;

      static constexpr std::array<const char[VK_MAX_EXTENSION_NAME_SIZE], 8>
         supported_instance_exts{"VK_KHR_surface", "VK_KHR_get_surface_capabilities2",
            "VK_KHR_wayland_surface", "VK_KHR_win32_surface", "VK_KHR_xcb_surface",
            "VK_KHR_xlib_surface", "VK_EXT_debug_utils", "VK_KHR_get_physical_device_properties2"};

      std::vector<std::string_view> available_instance_exts;
      std::vector<std::string_view> enabled_instance_exts;

      inline static bool IS_VOLK_INIT = false;
   };
} // namespace core::vk
