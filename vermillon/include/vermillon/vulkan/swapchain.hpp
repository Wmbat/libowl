#pragma once

#include <vermillon/vulkan/core.hpp>
#include <vermillon/vulkan/device.hpp>

namespace vkn
{
   /**
    * Holds all data related the vulkan swapchain, including images and
    * image views. May only be built using the inner builder class.
    */
   class swapchain final
   {
      static constexpr size_t expected_image_count = 3U;

      template <typename any_>
      using image_dynamic_array = crl::small_dynamic_array<any_, expected_image_count>;

   public:
      using value_type = vk::SwapchainKHR;
      using pointer = value_type*;
      using const_pointer = const value_type*;

      /**
       * Contains all possible error values comming from the swapchain class.
       */
      enum class error_type
      {
         surface_handle_not_provided,
         failed_to_query_surface_support_details,
         failed_to_create_swapchain,
         failed_to_get_swapchain_images,
         failed_to_create_swapchain_image_views,
      };

   public:
      swapchain() = default;

      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() noexcept -> pointer;
      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() const noexcept -> const_pointer;

      /**
       * Get the underlying handle
       */
      auto operator*() const noexcept -> value_type;

      operator bool() const noexcept; // NOLINT

      /**
       * Get the underlying handle
       */
      [[nodiscard]] auto value() const noexcept -> value_type;
      /**
       * Get the swapchain's image format
       */
      [[nodiscard]] auto format() const noexcept -> vk::Format;
      /**
       * Get the dimensions of the swapchain images
       */
      [[nodiscard]] auto extent() const noexcept -> const vk::Extent2D&;
      [[nodiscard]] auto image_views() const noexcept
         -> const crl::small_dynamic_array<vk::UniqueImageView, expected_image_count>&;

   private:
      vk::UniqueSwapchainKHR m_swapchain{nullptr};
      vk::Format m_format{};
      vk::Extent2D m_extent{};

      crl::small_dynamic_array<vk::Image, expected_image_count> m_images;
      crl::small_dynamic_array<vk::UniqueImageView, expected_image_count> m_image_views;

   public:
      /**
       * A class to help in the construction of a swapchain object.
       */
      class builder
      {
      public:
         builder(const device& device, cacao::logger_wrapper logger);

         /**
          * Attempt to build a swapchain object. May return an error
          */
         [[nodiscard]] auto build() -> util::result<swapchain>;

         auto set_old_swapchain(const swapchain& swap) noexcept -> builder&;

         auto set_desired_extent(uint32_t width, uint32_t height) noexcept -> builder&;

         auto set_desired_format(const vk::SurfaceFormatKHR& format) -> builder&;
         auto add_fallback_format(const vk::SurfaceFormatKHR& format) -> builder&;
         auto use_default_format_selection() -> builder&;

         auto set_desired_present_mode(vk::PresentModeKHR present_mode) -> builder&;
         auto add_fallback_present_mode(vk::PresentModeKHR present_mode) -> builder&;
         auto use_default_present_mode_selection() -> builder&;

         auto set_image_usage_flags(const vk::ImageUsageFlags& usage_flags) noexcept -> builder&;
         auto add_image_usage_flags(const vk::ImageUsageFlags& usage_flags) noexcept -> builder&;
         auto use_default_image_usage_flags() noexcept -> builder&;

         auto set_composite_alpha_flags(vk::CompositeAlphaFlagBitsKHR alpha_flags) noexcept
            -> builder&;

         auto set_clipped(bool clipped = true) noexcept -> builder&;

      private:
         [[nodiscard]] auto add_desired_formats() const
            -> crl::small_dynamic_array<vk::SurfaceFormatKHR, 2>;
         [[nodiscard]] auto add_desired_present_modes() const
            -> crl::small_dynamic_array<vk::PresentModeKHR, 2>;

         [[nodiscard]] auto create_swapchain(vk::SwapchainCreateInfoKHR&& info) const
            -> util::result<vk::UniqueSwapchainKHR>;
         [[nodiscard]] auto create_images(vk::SwapchainKHR swapchain) const
            -> util::result<image_dynamic_array<vk::Image>>;
         [[nodiscard]] auto create_image_views(const image_dynamic_array<vk::Image>& images,
                                               vk::SurfaceFormatKHR format) const
            -> util::result<image_dynamic_array<vk::UniqueImageView>>;

      private:
         static constexpr uint32_t DEFAULT_SIZE = 256;

         cacao::logger_wrapper m_logger;

         struct swap_info
         {
            vk::PhysicalDevice physical_device{nullptr};
            vk::Device device{nullptr};
            vk::SurfaceKHR surface{nullptr};
            vk::SwapchainKHR old_swapchain{nullptr};

            crl::small_dynamic_array<vk::SurfaceFormatKHR, 2> desired_formats{};
            crl::small_dynamic_array<vk::PresentModeKHR, 2> desired_present_modes{};

            uint32_t desired_width{DEFAULT_SIZE};
            uint32_t desired_height{DEFAULT_SIZE};

            uint32_t graphics_queue_index{0};
            uint32_t present_queue_index{0};

            vk::ImageUsageFlags image_usage_flags{vk::ImageUsageFlagBits::eColorAttachment};
            vk::CompositeAlphaFlagBitsKHR composite_alpha_flags{
               vk::CompositeAlphaFlagBitsKHR::eOpaque};

            vk::Bool32 clipped = VK_TRUE;
         } m_info;
      };

   private:
      /**
       * The information necessary for the creation of a swapchain
       */
      struct create_info
      {
         vk::UniqueSwapchainKHR swapchain{nullptr};
         vk::Format format{};
         vk::Extent2D extent{};

         crl::small_dynamic_array<vk::Image, expected_image_count> images{};
         crl::small_dynamic_array<vk::UniqueImageView, expected_image_count> image_views{};
      };

      explicit swapchain(create_info&& info) noexcept;

      /**
       * A struct used for error handling and displaying error messages
       */
      struct error_category : std::error_category
      {
         /**
          * The name of the vkn object the error appeared from.
          */
         [[nodiscard]] auto name() const noexcept -> const char* override;
         /**
          * Get the message associated with a specific error code.
          */
         [[nodiscard]] auto message(int err) const -> std::string override;
      };

      inline static const error_category m_category{};

      static auto make_error(swapchain::error_type flag) -> util::error_t
      {
         return {{static_cast<int>(flag), m_category}};
      };

      static auto make_error_res(swapchain::error_type flag) -> monad::error_t<util::error_t>
      {
         return {make_error(flag)};
      }
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::swapchain::error_type> : true_type
   {
   };
} // namespace std
