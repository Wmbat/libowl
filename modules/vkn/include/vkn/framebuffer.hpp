/**
 * @file command_pool.hpp
 * @author wmbat wmbat@protonmail.com
 * @date 9th of August, 2020
 * @copyright MIT License.
 */

#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>
#include <vkn/render_pass.hpp>

namespace vkn
{
   class framebuffer final
   {
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

   public:
      /**
       * Set of possible errors that may happen at construction of a framebuffer
       */
      enum struct error
      {
         no_device_handle,
         failed_to_create_framebuffer
      };

      struct create_info
      {
         vk::Device device{nullptr};
         vk::Framebuffer framebuffer{nullptr};

         vk::Extent2D dimensions{};
      };

   public:
      framebuffer() noexcept = default;
      framebuffer(const create_info& info) noexcept;
      framebuffer(create_info&& info) noexcept;
      framebuffer(const framebuffer&) noexcept = delete;
      framebuffer(framebuffer&& rhs) noexcept;
      ~framebuffer();

      auto operator=(const framebuffer&) noexcept = delete;
      auto operator=(framebuffer&& rhs) noexcept -> framebuffer&;

      [[nodiscard]] auto value() const noexcept -> vk::Framebuffer;
      /**
       * Get the device used for the creation of the framebuffer handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      /**
       * Turn an error flag and a standard error code into a vkn::error
       */
      inline static auto make_error(error err, std::error_code ec) -> vkn::error
      {
         return {{static_cast<int>(err), m_category}, static_cast<vk::Result>(ec.value())};
      }

   private:
      vk::Framebuffer m_framebuffer{nullptr};
      vk::Device m_device{nullptr};

      vk::Extent2D m_dimensions{};

   public:
      /**
       * A helper class to facilitate the construction of a framebuffer object.
       */
      class builder
      {
      public:
         builder(const vkn::device& device, const vkn::render_pass& render_pass,
                 util::logger* plogger) noexcept;

         /**
          * Attemp to create the framebuffer object with the given information. If something
          * unexpected happens, an error will be returned instead
          */
         auto build() -> vkn::result<framebuffer>;

         /**
          * Add a view attachment to the framebuffer
          */
         auto add_attachment(vk::ImageView image_view) noexcept -> builder&;
         /**
          * Set multiple image view attachments for the framebuffer
          */
         auto set_attachments(const util::dynamic_array<vk::ImageView>& attachments) -> builder&;
         /**
          * Set the width of the framebuffer
          */
         auto set_buffer_width(uint32_t width) noexcept -> builder&;
         /**
          * Set the height of the framebuffer
          */
         auto set_buffer_height(uint32_t height) noexcept -> builder&;
         /**
          * Set the number layers in the image array
          */
         auto set_layer_count(uint32_t count) noexcept -> builder&;

      private:
         vk::Device m_device{nullptr};
         util::logger* const m_plogger{nullptr};

         struct info
         {
            vk::RenderPass render_pass{nullptr};

            util::dynamic_array<vk::ImageView> attachments;

            uint32_t width{0};
            uint32_t height{0};
            uint32_t layer_count{0};
         } m_info;
      };
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::framebuffer::error> : true_type
   {
   };
} // namespace std
