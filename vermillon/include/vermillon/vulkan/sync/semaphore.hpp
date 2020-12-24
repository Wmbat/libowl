#pragma once

#include <vermillon/vulkan/core.hpp>
#include <vermillon/vulkan/device.hpp>

namespace vkn
{
   /**
    * The possible errors that may occur the construction of the
    * semaphore object
    */
   enum struct semaphore_error
   {
      failed_to_create_semaphore
   };

   /**
    * Convert an semaphore_error enum to a string
    */
   auto to_string(semaphore_error err) -> std::string;
   auto to_err_code(semaphore_error err) -> util::error_t;

   /**
    * Wrapper class around the vulkan semaphore handle. May only
    * be built uing the inner builder class.
    */
   class semaphore final : public owning_handle<vk::Semaphore>
   {
   public:
      /**
       * Get the device used to create the underlying handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   public:
      /**
       * Helper class to simplify the building of a semaphore object
       */
      class builder
      {
      public:
         builder(const vkn::device& device, cacao::logger_wrapper logger) noexcept;

         /**
          * Attempt to create the semaphore object. Returns an error
          * otherwise
          */
         [[nodiscard]] auto build() noexcept -> util::result<semaphore>;

      private:
         cacao::logger_wrapper m_logger;

         struct info
         {
            vk::Device device;
         } m_info;
      };
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::semaphore_error> : true_type
   {
   };
} // namespace std
