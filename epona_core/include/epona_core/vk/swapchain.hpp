#pragma once

#include "epona_core/vk/detail/includes.hpp"
#include "epona_core/vk/detail/result.hpp"
#include "epona_core/vk/device.hpp"

#include <system_error>
#include <type_traits>

namespace core::vk
{
   /**
    * @class swapchain swapchain.hpp <epona_core/vk/swapchain.hpp>
    * @author wmbat wmbat@protonmail.com
    * @date Monday, June 6th, 2020
    * @copyright MIT License.
    */
   struct swapchain
   {
   public:
      enum class error
      {
         surface_handle_not_provided
      };

   public:
      swapchain() = default;
      swapchain(const swapchain& other) = delete;
      swapchain(swapchain&& other) noexcept;
      ~swapchain();

      swapchain& operator=(const swapchain& rhs) = delete;
      swapchain& operator=(swapchain&& rhs) noexcept;

   public:
      static std::string to_string(error err);
      static std::error_code make_error_code(error err);

   public:
      VkDevice vk_device{VK_NULL_HANDLE};
      VkSwapchainKHR vk_swapchain{VK_NULL_HANDLE};
   };

   class swapchain_builder
   {
   public:
      swapchain_builder(const device& device, logger* const p_logger = nullptr);

      detail::result<swapchain> build() const;

      swapchain_builder& set_desired_format(VkSurfaceFormatKHR format) noexcept;
      swapchain_builder& add_fallback_format(VkSurfaceFormatKHR format) noexcept;
      swapchain_builder& use_default_format() noexcept;

      swapchain_builder& set_desired_present_mode(VkPresentModeKHR present_mode) noexcept;
      swapchain_builder& add_fallback_format(VkPresentModeKHR present_mode) noexcept;
      swapchain_builder& use_default_present_mode() noexcept;

   private:
      logger* const p_logger;

      struct swap_info
      {
         VkPhysicalDevice physical_device{VK_NULL_HANDLE};
         VkDevice device{VK_NULL_HANDLE};
         VkSurfaceKHR surface{VK_NULL_HANDLE};

         uint32_t graphics_queue_index{0};
         uint32_t present_queue_index{0};

         dynamic_array<VkSurfaceFormatKHR> formats;
         dynamic_array<VkPresentModeKHR> present_modes;

      } info;
   };
} // namespace core::vk

namespace std
{
   template <>
   struct is_error_code_enum<core::vk::swapchain::error> : true_type
   {
   };
} // namespace std
