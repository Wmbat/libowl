#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

namespace vkn
{
   /**
    * Wrapper class around the vulkan semaphore handle. May only
    * be built uing the inner builder class.
    */
   class semaphore final
   {
   public:
      using value_type = vk::Semaphore;
      using pointer = vk::Semaphore*;
      using const_pointer = const vk::Semaphore*;

      enum struct error
      {
         failed_to_create_semaphore
      };

   public:
      semaphore() = default;

      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() noexcept -> pointer;
      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() const noexcept -> const_pointer;

      /**
       * Get the underlying handle
       */
      auto operator*() const noexcept -> value_type;

      operator bool() const noexcept;

      /**
       * Get the underlying handle
       */
      [[nodiscard]] auto value() const noexcept -> value_type;
      /**
       * Get the device used to create the underlying handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::UniqueSemaphore m_semaphore{nullptr};

   public:
      /**
       * Helper class to simplify the building of a semaphore object
       */
      class builder
      {
      public:
         builder(const vkn::device& device, util::logger* p_logger) noexcept;

         /**
          * Attempt to create the semaphore object. Returns an error
          * otherwise
          */
         [[nodiscard]] auto build() const noexcept -> vkn::result<semaphore>;

      private:
         util::logger* const mp_logger;

         struct info
         {
            vk::Device device;
         } m_info;
      };

   private:
      struct create_info
      {
         vk::UniqueSemaphore semaphore;
      };

      semaphore(create_info&& info) noexcept;

      struct error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override;
         [[nodiscard]] auto message(int err) const -> std::string override;
      };

      inline static const error_category m_category{};

      inline static auto make_error(error err, std::error_code ec) -> vkn::error
      {
         return {{static_cast<int>(err), m_category}, static_cast<vk::Result>(ec.value())};
      }
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::semaphore::error> : true_type
   {
   };
} // namespace std
