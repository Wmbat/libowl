#include <vkn/framebuffer.hpp>

#include <monads/try.hpp>

namespace vkn
{
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
   auto make_error(framebuffer_error err, std::error_code ec) -> vkn::error
   {
      return {{static_cast<int>(err), framebuffer_category}, static_cast<vk::Result>(ec.value())};
   }

   auto framebuffer::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   using builder = framebuffer::builder;

   builder::builder(const vkn::device& device, const vkn::render_pass& render_pass,
                    std::shared_ptr<util::logger> p_logger) noexcept :
      m_device{device.value()},
      mp_logger{std::move(p_logger)}
   {
      m_info.render_pass = render_pass.value();
   }

   auto builder::build() -> vkn::result<framebuffer>
   {
      if (!m_device)
      {
         return monad::make_error(make_error(framebuffer_error::no_device_handle, {}));
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
         .map_error([](vk::SystemError&& err) {
            return make_error(framebuffer_error::failed_to_create_framebuffer, err.code());
         })
         .map([&](vk::UniqueFramebuffer&& handle) {
            util::log_info(mp_logger, "[vkn] framebuffer created");

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
} // namespace vkn
