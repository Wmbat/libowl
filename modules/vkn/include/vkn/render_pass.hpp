#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>
#include <vkn/swapchain.hpp>

namespace vkn
{
   class render_pass final
   {
   public:
      enum struct error
      {
         no_device_provided,
         failed_to_create_render_pass
      };

   private:
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

   public:
      struct create_info
      {
         vk::Device device{nullptr};
         vk::RenderPass render_pass{nullptr};
         vk::Format format{};
      };

      render_pass() noexcept = default;
      render_pass(create_info&& info) noexcept;
      render_pass(const render_pass&) = delete;
      render_pass(render_pass&& other) noexcept;
      ~render_pass();

      auto operator=(const render_pass&) -> render_pass& = delete;
      auto operator=(render_pass&& rhs) noexcept -> render_pass&;

      [[nodiscard]] auto value() const noexcept -> vk::RenderPass;
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::RenderPass m_render_pass{nullptr};
      vk::Device m_device{nullptr};
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
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::render_pass::error> : true_type
   {
   };
} // namespace std
