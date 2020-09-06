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
   /**
    * Set of possible errors that may happen at construction of a framebuffer
    */
   enum struct framebuffer_error
   {
      no_device_handle,
      failed_to_create_framebuffer
   };

   class framebuffer final : public owning_handle<vk::Framebuffer>
   {
      /**
       * Get the device used for the creation of the framebuffer handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::Extent2D m_dimensions{};

   public:
      /**
       * A helper class to facilitate the construction of a framebuffer object.
       */
      class builder
      {
      public:
         builder(const vkn::device& device, const vkn::render_pass& render_pass,
                 std::shared_ptr<util::logger> p_logger) noexcept;

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
         std::shared_ptr<util::logger> mp_logger{nullptr};

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

   /**
    * Convert an framebuffer_error enum to a string
    */
   auto to_string(vkn::framebuffer_error err) -> std::string;
   /**
    * Convert an framebuffer_error enum value and an error code from a vulkan error into
    * a vkn::error
    */
   auto make_error(framebuffer_error err, std::error_code ec) -> vkn::error;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::framebuffer_error> : true_type
   {
   };
} // namespace std
