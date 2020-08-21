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

   semaphore::semaphore(create_info&& info) noexcept :
      m_semaphore{info.semaphore}, m_device{info.device}
   {}
   semaphore::semaphore(semaphore&& other) noexcept { *this = std::move(other); }
   semaphore::~semaphore()
   {
      if (m_device && m_semaphore)
      {
         m_device.destroySemaphore(m_semaphore);
         m_semaphore = nullptr;
         m_device = nullptr;
      }
   }

   auto semaphore::operator=(semaphore&& rhs) noexcept -> semaphore&
   {
      std::swap(m_device, rhs.m_device);
      std::swap(m_semaphore, rhs.m_semaphore);

      return *this;
   }

   auto semaphore::value() const noexcept -> vk::Semaphore { return m_semaphore; }
   auto semaphore::device() const noexcept -> vk::Device { return m_device; }

   using builder = semaphore::builder;

   builder::builder(const vkn::device& device, util::logger* p_logger) noexcept :
      mp_logger{p_logger}
   {
      m_info.device = device.value();
   }

   auto builder::build() const noexcept -> vkn::result<semaphore>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createSemaphore({});
             })
         .left_map([](vk::SystemError&& err) {
            return make_error(error::failed_to_create_semaphore, err.code());
         })
         .right_map([&](vk::Semaphore&& handle) {
            util::log_info(mp_logger, "[vkn] semaphore semaphore");

            return semaphore{{.device = m_info.device, .semaphore = handle}};
         });
   }
} // namespace vkn
