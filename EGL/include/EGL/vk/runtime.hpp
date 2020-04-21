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

#include <EGL/vk/core.hpp>
#include <ESL/allocators/multipool_allocator.hpp>
#include <ESL/containers/vector.hpp>
#include <ESL/utils/logger.hpp>

#include <string>
#include <string_view>

namespace EGL::vk
{
   class runtime
   {
   public:
      runtime( ESL::multipool_allocator* p_main_allocator );
      runtime( std::string_view app_name_in, ESL::multipool_allocator* p_main_allocator, ESL::logger* p_log = nullptr );
      runtime( runtime const& other ) = delete;
      runtime( runtime&& other );
      ~runtime( );

      runtime& operator=( runtime const& rhs ) = delete;
      runtime& operator=( runtime&& rhs );

      runtime&& create_instance( );

   private:
      bool check_validation_layer_support( );

      ESL::vector<char const*> get_instance_extensions( );

   private:
      ESL::logger* p_log{ nullptr };
      ESL::multipool_allocator* p_main_allocator;

      std::uint32_t api_version{ 0 };
      std::string app_name{ };

      VkInstance instance{ VK_NULL_HANDLE };
      VkDebugUtilsMessengerEXT debug_messenger{ VK_NULL_HANDLE };

      ESL::vector<char const*> const validation_layer{ 1, "VK_LAYER_KHRONOS_validation", p_main_allocator };
      ESL::vector<char const*> instance_extensions{ p_main_allocator };
   };
} // namespace EGL::vk
