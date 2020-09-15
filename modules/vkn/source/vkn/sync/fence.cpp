#include <vkn/sync/fence.hpp>

#include <monads/try.hpp>

namespace vkn
{
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
   auto make_error(fence_error err, std::error_code ec) -> vkn::error
   {
      return {{static_cast<int>(err), fence_category}, static_cast<vk::Result>(ec.value())};
   }

   auto fence::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   using builder = fence::builder;

   builder::builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) :
      mp_logger{std::move(p_logger)}
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
         .map_error([](vk::SystemError&& err) {
            return make_error(fence_error::failed_to_create_fence, err.code());
         })
         .map([&](vk::UniqueFence&& handle) {
            util::log_info(mp_logger, "[vkn] {} fence created",
                           m_info.signaled ? "signaled" : "unsignaled");

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
} // namespace vkn
