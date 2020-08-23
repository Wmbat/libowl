#include <vkn/sync/fence.hpp>

#include <monads/try.hpp>

namespace vkn
{
   namespace detail
   {
      auto to_string(fence::error err) -> std::string
      {
         switch (err)
         {
            case fence::error::failed_to_create_fence:
               return "failed_to_create_fence";
            default:
               return "UNKNOWN";
         }
      }
   } // namespace detail

   auto fence::error_category::name() const noexcept -> const char* { return "vkn_fence"; }
   auto fence::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<fence::error>(err));
   }

   fence::fence(create_info&& info) noexcept : m_fence{std::move(info.fence)} {}

   auto fence::operator->() noexcept -> pointer { return &m_fence.get(); }
   auto fence::operator->() const noexcept -> const_pointer { return &m_fence.get(); }

   auto fence::operator*() const noexcept -> value_type { return value(); }

   fence::operator bool() const noexcept { return m_fence.get(); }

   auto fence::value() const noexcept -> vk::Fence { return m_fence.get(); }
   auto fence::device() const noexcept -> vk::Device { return m_fence.getOwner(); }

   using builder = fence::builder;

   builder::builder(const vkn::device& device, util::logger* p_logger) : mp_logger{p_logger}
   {
      m_info.device = device.value();
   }

   auto builder::build() const noexcept -> vkn::result<fence>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createFenceUnique({.pNext = nullptr,
                                                        .flags = m_info.signaled
                                                           ? vk::FenceCreateFlagBits::eSignaled
                                                           : vk::FenceCreateFlagBits{}});
             })
         .left_map([](vk::SystemError&& err) {
            return make_error(error::failed_to_create_fence, err.code());
         })
         .right_map([&](vk::UniqueFence&& handle) {
            return fence{{.fence = std::move(handle)}};
         });
   }

   auto builder::set_signaled(bool signaled) noexcept -> builder&
   {
      m_info.signaled = signaled;
      return *this;
   }
} // namespace vkn
