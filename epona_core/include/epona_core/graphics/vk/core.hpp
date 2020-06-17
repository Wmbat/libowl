#pragma once

#include "epona_core/details/monad/either.hpp"

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

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

      struct error
      {
         std::error_code type;
         ::vk::Result result;
      };

      template <class any_>
      using result = either<error, any_>;
   } // namespace detail

   class loader
   {
   public:
   public:
      loader();

      void load_instance(const ::vk::Instance& instance) const;
      void load_device(const ::vk::Device& device) const;
   };
}; // namespace core::gfx::vkn
