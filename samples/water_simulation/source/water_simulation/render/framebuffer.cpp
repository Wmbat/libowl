#include <water_simulation/render/framebuffer.hpp>

auto framebuffer::make(create_info&& info) -> result<framebuffer>
{
   const auto create_info = vk::FramebufferCreateInfo{}
                               .setPNext(nullptr)
                               .setFlags({})
                               .setRenderPass(info.pass)
                               .setAttachmentCount(info.attachments.size())
                               .setPAttachments(info.attachments.data())
                               .setWidth(info.width)
                               .setHeight(info.height)
                               .setLayers(info.layers);

   return monad::try_wrap<vk::SystemError>([&] {
             return info.device.createFramebufferUnique(create_info);
          })
      .map_error([]([[maybe_unused]] auto err) {
         return to_err_code(framebuffer_error::failed_to_create_framebuffer);
      })
      .map([&](vk::UniqueFramebuffer&& handle) {
         util::log_info(info.logger, "framebuffer created");

         framebuffer f{};
         f.m_framebuffer = std::move(handle);
         f.m_width = info.width;
         f.m_height = info.height;
         f.m_layers = info.layers;
         return f;
      });
}

auto framebuffer::value() const -> vk::Framebuffer
{
   return m_framebuffer.get();
}

struct framebuffer_error_category : std::error_category
{
   /**
    * The name of the vkn object the error appeared from.
    */
   [[nodiscard]] auto name() const noexcept -> const char* override
   {
      return "framebuffer";
   } // namespace vkn
   /**
    * Get the message associated with a specific error code.
    */
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return to_string(static_cast<framebuffer_error>(err));
   }
};

inline static const framebuffer_error_category framebuffer_error_cat{};

auto to_string(framebuffer_error err) -> std::string
{
   if (err == framebuffer_error::failed_to_create_framebuffer)
   {
      return "failed_to_create_framebuffer";
   }

   return "UNKNOWN";
}
auto to_err_code(framebuffer_error err) -> util::error_t
{
   return {{static_cast<int>(err), framebuffer_error_cat}};
}
