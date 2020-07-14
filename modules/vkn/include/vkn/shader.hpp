/**
 * @file swapchain.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of June, 2020
 * @copyright MIT License
 */

#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

namespace vkn
{
   class shader
   {
   public:
      class builder
      {
      public:
         builder(const device& device, util::logger* const plogger);

         [[nodiscard]] auto build() -> result<shader>;

      private:
      };
   };
} // namespace vkn
