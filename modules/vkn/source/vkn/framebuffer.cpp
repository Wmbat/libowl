#include <vkn/framebuffer.hpp>

#include <monads/try.hpp>

namespace vkn
{
   auto framebuffer::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   using builder = framebuffer::builder;

   builder::builder(const vkn::device& device, const vkn::render_pass& render_pass,
                    util::logger_wrapper logger) noexcept :
      m_device{device.logical()},
      m_logger{logger}
   {
      m_info.render_pass = render_pass.value();
   }

   auto builder::build() -> util::result<framebuffer>
   {
      if (!m_device)
      {
         return monad::err(to_err_code(framebuffer_error::no_device_handle));
      }

      const auto create_info = vk::FramebufferCreateInfo{}
                                  .setPNext(nullptr)
                                  .setFlags({})
                                  .setRenderPass(m_info.render_pass)
                                  .setAttachmentCount(m_info.attachments.size())
                                  .setPAttachments(m_info.attachments.data())
                                  .setWidth(m_info.width)
                                  .setHeight(m_info.height)
                                  .setLayers(m_info.layer_count);

      return monad::try_wrap<vk::SystemError>([&] {
                return m_device.createFramebufferUnique(create_info);
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(framebuffer_error::failed_to_create_framebuffer);
         })
         .map([&](vk::UniqueFramebuffer&& handle) {
            m_logger.info("[vulkan] framebuffer created");

            framebuffer f{};
            f.m_value = std::move(handle);
            f.m_dimensions = {m_info.width, m_info.height};

            return f;
         });
   }

   auto builder::add_attachment(vk::ImageView image_view) noexcept -> builder&
   {
      m_info.attachments.emplace_back(image_view);
      return *this;
   }
   auto builder::set_attachments(const util::dynamic_array<vk::ImageView>& attachments) -> builder&
   {
      m_info.attachments = attachments;
      return *this;
   }
   auto builder::set_buffer_width(uint32_t width) noexcept -> builder&
   {
      m_info.width = width;
      return *this;
   }
   auto builder::set_buffer_height(uint32_t height) noexcept -> builder&
   {
      m_info.height = height;
      return *this;
   }
   auto builder::set_layer_count(uint32_t count) noexcept -> builder&
   {
      m_info.layer_count = count;
      return *this;
   }

   /**
    * A struct used for error handling and displaying error messages
    */
   struct framebuffer_error_category : std::error_category
   {
      /**
       * The name of the vkn object the error appeared from.
       */
      [[nodiscard]] auto name() const noexcept -> const char* override { return "vkn_framebuffer"; }
      /**
       * Get the message associated with a specific error code.
       */
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<framebuffer_error>(err));
      }
   };

   inline static const framebuffer_error_category framebuffer_category{};

   auto to_string(vkn::framebuffer_error err) -> std::string
   {
      switch (err)
      {
         case framebuffer_error::no_device_handle:
            return "no_device_handle";
         case framebuffer_error::failed_to_create_framebuffer:
            return "failed_to_create_framebuffer";
         default:
            return "UNKNOWN";
      }
   };

   auto to_err_code(framebuffer_error err) -> util::error_t
   {
      return {{static_cast<int>(err), framebuffer_category}};
   }
} // namespace vkn
