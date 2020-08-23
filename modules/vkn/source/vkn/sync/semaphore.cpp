#include <vkn/sync/semaphore.hpp>

#include <monads/try.hpp>

namespace vkn
{
   namespace detail
   {
      auto to_string(semaphore::error err) -> std::string
      {
         switch (err)
         {
            case semaphore::error::failed_to_create_semaphore:
               return "failed_to_create_semaphore";
            default:
               return "UNKNOWN";
         }
      }
   } // namespace detail

   auto semaphore::error_category::name() const noexcept -> const char* { return "vkn_semaphore"; }
   auto semaphore::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<semaphore::error>(err));
   }

   semaphore::semaphore(create_info&& info) noexcept : m_semaphore{std::move(info.semaphore)} {}

   auto semaphore::operator->() noexcept -> pointer { return &m_semaphore.get(); }
   auto semaphore::operator->() const noexcept -> const_pointer { return &m_semaphore.get(); }

   auto semaphore::operator*() const noexcept -> value_type { return value(); }

   semaphore::operator bool() const noexcept { return m_semaphore.get(); }

   auto semaphore::value() const noexcept -> value_type { return m_semaphore.get(); }
   auto semaphore::device() const noexcept -> vk::Device { return m_semaphore.getOwner(); }

   using builder = semaphore::builder;

   builder::builder(const vkn::device& device, util::logger* p_logger) noexcept :
      mp_logger{p_logger}
   {
      m_info.device = device.value();
   }

   auto builder::build() const noexcept -> vkn::result<semaphore>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createSemaphoreUnique({});
             })
         .left_map([](vk::SystemError&& err) {
            return make_error(error::failed_to_create_semaphore, err.code());
         })
         .right_map([&](vk::UniqueSemaphore&& handle) {
            util::log_info(mp_logger, "[vkn] semaphore semaphore");

            return semaphore{{.semaphore = std::move(handle)}};
         });
   }
} // namespace vkn
