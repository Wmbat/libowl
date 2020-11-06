#include <vkn/render_pass.hpp>

#include <monads/try.hpp>

namespace vkn
{
   /**
    * A struct used for error handling and displaying error messages
    */
   struct render_pass_error_category : std::error_category
   {
      /**
       * The name of the vkn object the error appeared from.
       */
      [[nodiscard]] auto name() const noexcept -> const char* override { return "vkn_render_pass"; }
      /**
       * Get the message associated with a specific error code.
       */
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<render_pass_error>(err));
      }
   };

   inline static const render_pass_error_category render_pass_category{};

   auto make_error(render_pass_error err) -> vkn::error_t
   {
      return {{static_cast<int>(err), render_pass_category}};
   }
   auto to_string(render_pass_error err) -> std::string
   {
      switch (err)
      {
         case render_pass_error::no_device_provided:
            return "no_device_provided";
         case render_pass_error::failed_to_create_render_pass:
            return "failed_to_create_render_pass";
         default:
            return "UNKNOWN";
      }
   };

   auto render_pass::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   using builder = render_pass::builder;

   builder::builder(const vkn::device& device, const vkn::swapchain& swapchain,
                    std::shared_ptr<util::logger> p_logger) noexcept :
      m_device{device.logical_device()},
      m_swapchain_format{swapchain.format()},
      m_swapchain_extent{swapchain.extent()}, mp_logger{std::move(p_logger)}
   {}

   auto builder::build() -> vkn::result<render_pass>
   {
      if (!m_device)
      {
         return monad::err(make_error(render_pass_error::no_device_provided));
      }

      const std::array attachment_descriptions{
         vk::AttachmentDescription{.flags = {},
                                   .format = m_swapchain_format,
                                   .loadOp = vk::AttachmentLoadOp::eClear,
                                   .storeOp = vk::AttachmentStoreOp::eStore,
                                   .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                                   .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                                   .initialLayout = vk::ImageLayout::eUndefined,
                                   .finalLayout = vk::ImageLayout::ePresentSrcKHR}};

      const std::array attachment_references{vk::AttachmentReference{
         .attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal}};

      const std::array subpass_descriptions{
         vk::SubpassDescription{.flags = {},
                                .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                                .inputAttachmentCount = 0U,
                                .pInputAttachments = nullptr,
                                .colorAttachmentCount = attachment_references.size(),
                                .pColorAttachments = attachment_references.data(),
                                .pResolveAttachments = nullptr,
                                .pDepthStencilAttachment = nullptr,
                                .preserveAttachmentCount = 0U,
                                .pPreserveAttachments = nullptr}};

      const std::array subpass_dependencies{
         vk::SubpassDependency{.srcSubpass = VK_SUBPASS_EXTERNAL,
                               .dstSubpass = 0,
                               .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                               .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                               .srcAccessMask = {},
                               .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
                               .dependencyFlags = {}}};

      const auto pass_info = vk::RenderPassCreateInfo{}
                                .setPNext(nullptr)
                                .setFlags({})
                                .setDependencyCount(std::size(subpass_dependencies))
                                .setPDependencies(std::data(subpass_dependencies))
                                .setAttachmentCount(std::size(attachment_descriptions))
                                .setPAttachments(std::data(attachment_descriptions))
                                .setSubpassCount(std::size(subpass_descriptions))
                                .setPSubpasses(std::data(subpass_descriptions));

      return monad::try_wrap<vk::SystemError>([&] {
                return m_device.createRenderPassUnique(pass_info);
             })
         .map_error([]([[maybe_unused]] auto&& err) {
            return make_error(render_pass_error::failed_to_create_render_pass);
         })
         .map([&](vk::UniqueRenderPass&& handle) {
            util::log_info(mp_logger, "[vkn] render pass created");

            render_pass pass{};
            pass.m_value = std::move(handle);
            pass.m_swapchain_format = m_swapchain_format;

            return pass;
         });
   } // namespace vkn
} // namespace vkn
