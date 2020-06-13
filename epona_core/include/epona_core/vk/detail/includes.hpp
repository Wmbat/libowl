/**
 * @file includes.hpp
 * @author wmbat wmbat@protonmail.com.
 * @date Tuesday, June 5th, 2020.
 * @copyright MIT License.
 */

#pragma once

#include "epona_core/containers/dynamic_array.hpp"
#include "epona_core/details/concepts.hpp"
#include "epona_core/details/monad/either.hpp"

#include <volk.h>

#include <GLFW/glfw3.h>

#include <string>

namespace core::vk::detail
{
#if defined(NDEBUG)
   static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
   static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

   template <class any_, class fun_, class... args_>
   auto get_array(const fun_& fun, args_&&... args) -> either<dynamic_array<any_>, VkResult>
   {
      uint32_t count = 0;
      if (auto res = fun(std::forward<args_>(args)..., &count, nullptr); res != VK_SUCCESS)
      {
         return monad::to_right(res);
      }
      dynamic_array<any_> data(count);
      if (auto res = fun(std::forward<args_>(args)..., &count, data.data()); res != VK_SUCCESS)
      {
         return monad::to_right(res);
      }

      return monad::to_left(data);
   }
} // namespace core::vk::detail
