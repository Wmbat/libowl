#include "epona_core/vk/runtime.hpp"
#include "epona_core/details/logger.hpp"

#include <ranges>

namespace core::vk
{
   runtime::runtime(logger* p_logger) : p_logger(p_logger)
   {
      if (auto result = volkInitialize(); result != VK_SUCCESS)
      {
         LOG_ERROR(p_logger, "Failed to initialize volk");

         abort();
         // error
      }

      LOG_INFO(p_logger, "Volk initialized");

      if (auto result = glfwInit(); result != GLFW_TRUE)
      {
         LOG_ERROR(p_logger, "Failed to initialize glfw");

         abort();
         // error
      }

      LOG_INFO(p_logger, "glfw initialized");

      api_version = volkGetInstanceVersion();

      uint32_t layer_count = 0;
      vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
      layers.resize(layer_count);
      vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

      validation_layers_available =
         std::ranges::find_if(layers, [](const VkLayerProperties& layer) {
            return strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0;
         }) != layers.end();

      uint32_t extension_count = 0;
      vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
      extensions.resize(extension_count);
      vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

      debug_utils_available =
         std::ranges::find_if(extensions, [](const VkExtensionProperties& ext) {
            return strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
         }) != extensions.end();
   }
} // namespace core::vk
