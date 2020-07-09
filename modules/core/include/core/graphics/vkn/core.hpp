/**
 * @file core.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 20th of April, 2020
 * @copyright MIT License.
 */

#pragma once

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#include <util/logger.hpp>
#include <util/monad/either.hpp>
#include <util/monad/result.hpp>

#include <vulkan/vulkan.hpp>

#include <system_error>

namespace core::gfx::vkn
{
   namespace detail
   {
#if defined(NDEBUG)
      static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
      static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif
   } // namespace detail

   /**
    * @class error <epona_core/graphics/vkn/core.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Saturday, 20th of June, 2020
    * @copyright MIT License
    *
    * @brief The default error type for all code within the vkn namespace.
    */
   struct error
   {
      std::error_code type;
      vk::Result result{};
   };

   /**
    * @brief An alias for an either monad using an error as the left type.
    */
   template <class any_>
   using result = util::result<error, any_>;

   /**
    * @class error <epona_core/graphics/vkn/core.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Saturday, 20th of June, 2020
    * @copyright MIT License
    *
    * @brief Class used for the dynamic loading of the Vulkan API functions.
    */
   class loader
   {
   public:
   public:
      loader(util::logger* const p_logger = nullptr);

      void load_instance(const ::vk::Instance& instance) const;
      void load_device(const ::vk::Device& device) const;

   private:
      util::logger* const p_logger;

      ::vk::DynamicLoader dynamic_loader;
   };
}; // namespace core::gfx::vkn
