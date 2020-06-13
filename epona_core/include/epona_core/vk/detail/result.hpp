/**
 * @file result.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, June 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/details/monad/either.hpp"
#include "epona_core/vk/detail/includes.hpp"

#include <cassert>
#include <system_error>

namespace core::vk::detail
{
   /**
    * @class error result.hpp "epona_core/vk/details/result.hpp"
    * @author wmbat wmbat@protonmail.com
    * @date Monday, June 6th, 2020
    * @copyright MIT License.
    * @brief A simple struct to handle a vulkan error.
    */
   struct error
   {
      std::error_code type;
      VkResult result = VK_SUCCESS;
   };

   template <typename any_>
   using result = either<any_, error>;

} // namespace core::vk::detail
