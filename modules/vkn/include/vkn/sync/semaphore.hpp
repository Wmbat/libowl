#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

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
         builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) noexcept;

         /**
          * Attempt to create the semaphore object. Returns an error
          * otherwise
          */
         [[nodiscard]] auto build() const noexcept -> vkn::result<semaphore>;

      private:
         std::shared_ptr<util::logger> mp_logger;

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
