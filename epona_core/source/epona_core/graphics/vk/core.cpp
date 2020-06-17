#include "epona_core/graphics/vk/core.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace core::gfx::vkn
{
   loader::loader()
   {
      ::vk::DynamicLoader loader;
      VULKAN_HPP_DEFAULT_DISPATCHER.init(
         loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));
   }

   void loader::load_instance(const ::vk::Instance &instance) const
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
   }

   void loader::load_device(const ::vk::Device &device) const
   {
      VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
   }
} // namespace core::gfx::vkn
