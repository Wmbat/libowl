/**
 * @file core.cpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of April, 2020
 * @copyright MIT License.
 */

#include "vkn/core.hpp"

#include <util/logger.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace vkn
{
   loader::loader(util::logger *const p_logger) : p_logger{p_logger}
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(
         dynamic_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

      log_info(p_logger, "[vkn] base functions have been loaded");

      if (!loader::IS_GLSLANG_INIT)
      {
         glslang::InitializeProcess();
         loader::IS_GLSLANG_INIT = true;

         util::log_info(p_logger, "[vkn] glslang initialized");
      }
   }

   void loader::load_instance(const ::vk::Instance &instance) const
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

      log_info(p_logger, "[vkn] all instance functions have been loaded");
   }

   void loader::load_device(const ::vk::Device &device) const
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

      log_info(p_logger, "[vkn] all device functions have been loaded");
   }
} // namespace vkn
