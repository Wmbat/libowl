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

#include "epona_core/vk/runtime.hpp"
#include "epona_core/details/logger.hpp"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <span>

namespace core::vk
{
   runtime::runtime(logger* p_logger) : p_logger(p_logger)
   {
      if (!IS_VOLK_INIT)
      {
         if (auto res = volkInitialize(); res != VK_SUCCESS)
         {
            LOG_ERROR(p_logger, "Failed to initialize volk");

            abort();
         }
         else
         {
            LOG_INFO(p_logger, "Volk Initialized");

            IS_VOLK_INIT = true;
         }

         version = volkGetInstanceVersion();

         std::uint32_t ext_count{0};
         vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
         std::vector<VkExtensionProperties> exts(ext_count);
         vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, exts.data());

         for (auto const& ext : exts)
         {
            for (auto const& supported : supported_instance_exts)
            {
               if (strcmp(ext.extensionName, supported_instance_exts[0]) == 0)
               {
                  auto it = std::find(
                     available_instance_exts.cbegin(), available_instance_exts.cend(), supported);
                  if (it == available_instance_exts.end())
                  {
                     available_instance_exts.emplace_back(supported);
                  }
               }
            }
         }
      }
   }

   auto runtime::create_instance(std::string_view app_name) -> void
   {
      std::uint32_t glfw_extension_count{0};
      [[maybe_unused]] char const** glfw_extensions_raw =
         glfwGetRequiredInstanceExtensions(&glfw_extension_count);

      LOG_INFO_P(p_logger, "Vulkan version: {1}.{2}.{3}", VK_VERSION_MAJOR(version),
         VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));

      VkApplicationInfo app_info{};
      app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pApplicationName = app_name.data();
      app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
      app_info.pEngineName = "Epona";
      app_info.engineVersion =
         VK_MAKE_VERSION(CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH);
      app_info.apiVersion = version;

      VkInstanceCreateInfo create_info{};
      create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      create_info.pNext = nullptr;
      create_info.flags = {};
      create_info.pApplicationInfo = &app_info;
      create_info.enabledExtensionCount = 0;
      create_info.ppEnabledExtensionNames = nullptr;

      if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
      {
         LOG_ERROR(p_logger, "Failed to create vulkan instance");

         abort();
         // handle exception or error
      }

      volkLoadInstance(instance);
   }
} // namespace core::vk
