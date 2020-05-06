/**
 * @file extension.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, May 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include <epona_core/vk/core.hpp>

namespace core::vk
{
   struct extension
   {
      char const* name[VK_MAX_EXTENSION_NAME_SIZE];
   };
}; // namespace core::vk
