#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/details/logger.hpp"
#include "epona_core/vk/core.hpp"

namespace core::vk
{
   struct runtime
   {
   public:
      runtime(logger* p_logger);

      uint32_t api_version;

      dynamic_array<VkLayerProperties> layers;
      dynamic_array<VkExtensionProperties> extensions;
      bool validation_layers_available = false;
      bool debug_utils_available = false;

   private:
      logger* p_logger = nullptr;
   };
} // namespace core::vk
