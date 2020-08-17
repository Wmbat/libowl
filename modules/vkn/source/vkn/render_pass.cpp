#include <vkn/render_pass.hpp>

#include <monads/try.hpp>

namespace vkn
{
   namespace detail
   {
      auto to_string(render_pass::error err) -> std::string
      {
         using error = render_pass::error;

         switch (err)
         {
            case error::no_device_provided:
               return "no_device_provided";
            case error::failed_to_create_render_pass:
               return "failed_to_create_render_pass";
            default:
               return "UNKNOWN";
         }
      };
   } // namespace detail

   auto render_pass::error_category::name() const noexcept -> const char*
   {
      return "vkn_render_pass";
   }
   auto render_pass::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<render_pass::error>(err));
   }

   render_pass::render_pass(const create_info& info) noexcept :
      m_device{info.device}, m_render_pass{info.render_pass}
   {}
   render_pass::render_pass(create_info&& info) noexcept :
      m_device{info.device}, m_render_pass{info.render_pass}
   {}
   render_pass::render_pass(render_pass&& other) noexcept { *this = std::move(other); }
   render_pass::~render_pass()
   {
      if (m_device && m_render_pass)
      {
         m_device.destroyRenderPass(m_render_pass);
         m_device = nullptr;
         m_render_pass = nullptr;
      }
   }

   auto render_pass::operator=(render_pass&& rhs) noexcept -> render_pass&
   {
      std::swap(m_device, rhs.m_device);
      std::swap(m_render_pass, rhs.m_render_pass);

      return *this;
   }

   auto render_pass::value() const noexcept -> vk::RenderPass { return m_render_pass; }
   auto render_pass::device() const noexcept -> vk::Device { return m_device; }

   using builder = render_pass::builder;

   builder::builder(const vkn::device& device, const vkn::swapchain& swapchain,
                    util::logger* plogger) noexcept :
      m_device{device.value()},
      m_swapchain_format{swapchain.format()}, m_plogger{plogger}
   {}

   auto builder::build() -> vkn::result<render_pass>
   {
      if (!m_device)
      {
         return monad::make_left(make_error(error::no_device_provided, {}));
      }

      const std::array<vk::AttachmentDescription, 1> attachment_descriptions{
         vk::AttachmentDescription{.flags = {},
                                   .format = m_swapchain_format,
                                   .loadOp = vk::AttachmentLoadOp::eClear,
                                   .storeOp = vk::AttachmentStoreOp::eStore,
                                   .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                                   .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                                   .initialLayout = vk::ImageLayout::eUndefined,
                                   .finalLayout = vk::ImageLayout::ePresentSrcKHR}};

      const std::array<vk::AttachmentReference, 1> attachment_references{vk::AttachmentReference{
         .attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal}};

      const std::array<vk::SubpassDescription, 1> subpass_descriptions{
         vk::SubpassDescription{.flags = {},
                                .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                                .inputAttachmentCount = 0u,
                                .pInputAttachments = nullptr,
                                .colorAttachmentCount = attachment_references.size(),
                                .pColorAttachments = attachment_references.data(),
                                .pResolveAttachments = nullptr,
                                .pDepthStencilAttachment = nullptr,
                                .preserveAttachmentCount = 0u,
                                .pPreserveAttachments = nullptr}};

      const auto pass_info = vk::RenderPassCreateInfo{}
                                .setPNext(nullptr)
                                .setFlags({})
                                .setDependencyCount(0)
                                .setPDependencies(nullptr)
                                .setAttachmentCount(attachment_descriptions.size())
                                .setPAttachments(attachment_descriptions.data())
                                .setSubpassCount(subpass_descriptions.size())
                                .setPSubpasses(subpass_descriptions.data());

      return monad::try_wrap<vk::SystemError>([&] {
                return m_device.createRenderPass(pass_info);
             })
         .left_map([](auto&& err) {
            return make_error(error::failed_to_create_render_pass, err.code());
         })
         .right_map([&](auto&& handle) {
            util::log_info(m_plogger, "[vkn] render pass created");

            return render_pass{{.device = m_device, .render_pass = handle}};
         });
   }
} // namespace vkn
