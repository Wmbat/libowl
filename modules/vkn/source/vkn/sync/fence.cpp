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

   fence::fence(create_info&& info) noexcept : m_fence{info.fence}, m_device{info.device} {}
   fence::fence(fence&& info) noexcept { *this = std::move(info); }
   fence::~fence()
   {
      if (m_device && m_fence)
      {
         m_device.destroyFence(m_fence);
         m_fence = nullptr;
         m_device = nullptr;
      }
   }

   auto fence::operator=(fence&& rhs) noexcept -> fence&
   {
      std::swap(m_device, rhs.m_device);
      std::swap(m_fence, rhs.m_fence);

      return *this;
   }

   auto fence::value() const noexcept -> vk::Fence { return m_fence; }
   auto fence::device() const noexcept -> vk::Device { return m_device; }

   using builder = fence::builder;

   builder::builder(const vkn::device& device, util::logger* p_logger) : mp_logger{p_logger}
   {
      m_info.device = device.value();
   }

   auto builder::build() const noexcept -> vkn::result<fence>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createFence({.pNext = nullptr,
                                                  .flags = m_info.signaled
                                                     ? vk::FenceCreateFlagBits::eSignaled
                                                     : vk::FenceCreateFlagBits{}});
             })
         .left_map([](vk::SystemError&& err) {
            return make_error(error::failed_to_create_fence, err.code());
         })
         .right_map([&](vk::Fence&& handle) {
            return fence{{.device = m_info.device, .fence = handle}};
         });
   }

   auto builder::set_signaled(bool signaled) noexcept -> builder&
   {
      m_info.signaled = signaled;
      return *this;
   }
} // namespace vkn
