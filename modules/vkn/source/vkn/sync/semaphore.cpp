#include <vkn/sync/semaphore.hpp>

#include <monads/try.hpp>

namespace vkn
{
   namespace detail
   {
   } // namespace detail

   struct semaphore_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override { return "vkn_semaphore"; }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<semaphore_error>(err));
      }
   };

   inline static const semaphore_error_category semaphore_category{};

   auto to_string(semaphore_error err) -> std::string
   {
      switch (err)
      {
         case semaphore_error::failed_to_create_semaphore:
            return "failed_to_create_semaphore";
         default:
            return "UNKNOWN";
      }
   }

   auto to_err_code(semaphore_error err) -> util::error_t
   {
      return {{static_cast<int>(err), semaphore_category}};
   }

   auto semaphore::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   using builder = semaphore::builder;

   builder::builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) noexcept :
      mp_logger{std::move(p_logger)}
   {
      m_info.device = device.logical_device();
   }

   auto builder::build() const noexcept -> util::result<semaphore>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createSemaphoreUnique({});
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(semaphore_error::failed_to_create_semaphore);
         })
         .map([&](vk::UniqueSemaphore&& handle) {
            util::log_info(mp_logger, "[vkn] semaphore created");

            semaphore s{};
            s.m_value = std::move(handle);

            return s;
         });
   }
} // namespace vkn
