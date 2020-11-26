#include <vkn/sync/semaphore.hpp>

#include <monads/try.hpp>

namespace vkn
{
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

   builder::builder(const vkn::device& device, util::logger_wrapper logger) noexcept :
      m_logger{logger}
   {
      m_info.device = device.logical();
   }

   auto builder::build() noexcept -> util::result<semaphore>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createSemaphoreUnique({});
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(semaphore_error::failed_to_create_semaphore);
         })
         .map([&](vk::UniqueSemaphore&& handle) {
            m_logger.info("[vulkan] semaphore created");

            semaphore s{};
            s.m_value = std::move(handle);

            return s;
         });
   }
} // namespace vkn
