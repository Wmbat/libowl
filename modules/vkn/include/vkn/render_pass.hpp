#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>
#include <vkn/swapchain.hpp>

namespace vkn
{
   class render_pass final
   {
   public:
      using value_type = vk::RenderPass;
      using pointer = vk::RenderPass*;
      using const_pointer = const vk::RenderPass*;

      enum struct error
      {
         no_device_provided,
         failed_to_create_render_pass
      };

   public:
      render_pass() noexcept = default;

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

      operator bool() const noexcept;

      /**
       * Get the underlying handle
       */
      [[nodiscard]] auto value() const noexcept -> value_type;
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::UniqueRenderPass m_render_pass{nullptr};
      vk::Format m_swapchain_format{};

   public:
      class builder
      {
      public:
         builder(const vkn::device& device, const vkn::swapchain& swapchain,
                 util::logger* plogger) noexcept;

         /**
          * Construct a render_pass object. If construction fails, an error will be
          * returned instead
          */
         auto build() -> vkn::result<render_pass>;

      private:
         vk::Device m_device;
         vk::Format m_swapchain_format;
         vk::Extent2D m_swapchain_extent;

         util::logger* const m_plogger;
      };

   private:
      struct create_info
      {
         vk::UniqueRenderPass render_pass{nullptr};
         vk::Format format{};
      };

      render_pass(create_info&& info) noexcept;

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

      /**
       * Turn an error flag and a standard error code into a vkn::error
       */
      inline static auto make_error(error err, std::error_code ec) -> vkn::error
      {
         return {{static_cast<int>(err), m_category}, static_cast<vk::Result>(ec.value())};
      }
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::render_pass::error> : true_type
   {
   };
} // namespace std
