#include "epona_core/graphics/vkn/core.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace core::gfx::vkn
{
   loader::loader(logger *const p_logger) : p_logger{p_logger}
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(
         dynamic_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

      LOG_INFO(p_logger, "vk - base functions have been loaded");
   }

   void loader::load_instance(const ::vk::Instance &instance) const
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

      LOG_INFO(p_logger, "vk - all instance functions have been loaded");
   }

   void loader::load_device(const ::vk::Device &device) const
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

      LOG_INFO(p_logger, "vk - all device functions have been loaded");
   }
} // namespace core::gfx::vkn
