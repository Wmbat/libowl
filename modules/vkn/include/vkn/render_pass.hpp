#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>
#include <vkn/swapchain.hpp>

namespace vkn
{
   enum struct render_pass_error
   {
      no_device_provided,
      failed_to_create_render_pass
   };

   class render_pass final : public owning_handle<vk::RenderPass>
   {
   public:
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::Format m_swapchain_format{};

   public:
      class builder
      {
      public:
         builder(const vkn::device& device, const vkn::swapchain& swapchain,
                 std::shared_ptr<util::logger> p_logger) noexcept;

         /**
          * Construct a render_pass object. If construction fails, an error will be
          * returned instead
          */
         auto build() -> vkn::result<render_pass>;

      private:
         vk::Device m_device;
         vk::Format m_swapchain_format;
         vk::Extent2D m_swapchain_extent;

         std::shared_ptr<util::logger> mp_logger;
      };
   };

   auto to_string(render_pass_error err) -> std::string;
   /**
    * Turn an render_pass_error and a standard error code into a vkn::error
    */
   auto make_error(render_pass_error err, std::error_code ec) -> vkn::error;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::render_pass_error> : true_type
   {
   };
} // namespace std
