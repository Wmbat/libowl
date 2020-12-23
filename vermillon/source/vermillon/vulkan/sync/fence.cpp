#include <vermillon/vulkan/sync/fence.hpp>

#include <monads/try.hpp>

namespace vkn
{
   auto fence::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   using builder = fence::builder;

   builder::builder(const vkn::device& device, util::logger_wrapper logger) : m_logger{logger}
   {
      m_info.device = device.logical();
   }

   auto builder::build() noexcept -> util::result<fence>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createFenceUnique(vk::FenceCreateInfo{}.setFlags(
                   m_info.signaled ? vk::FenceCreateFlagBits::eSignaled
                                   : vk::FenceCreateFlagBits{}));
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(fence_error::failed_to_create_fence);
         })
         .map([&](vk::UniqueFence&& handle) {
            m_logger.info("[vkn] {} fence created", m_info.signaled ? "signaled" : "unsignaled");

            fence f{};
            f.m_value = std::move(handle);

            return f;
         });
   }

   auto builder::set_signaled(bool signaled) noexcept -> builder&
   {
      m_info.signaled = signaled;
      return *this;
   }

   struct fence_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override { return "vkn_fence"; }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<fence_error>(err));
      }
   };

   inline static const fence_error_category fence_category{};

   auto to_string(fence_error err) -> std::string
   {
      switch (err)
      {
         case fence_error::failed_to_create_fence:
            return "failed_to_create_fence";
         default:
            return "UNKNOWN";
      }
   }

   auto to_err_code(fence_error err) -> util::error_t
   {
      return {{static_cast<int>(err), fence_category}};
   }

} // namespace vkn
