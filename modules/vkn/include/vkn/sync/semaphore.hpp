#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

namespace vkn
{
   /**
    * Wrapper class around the vulkan semphore handle
    */
   class semaphore final
   {
      /**
       * A struct used for error handling and displaying error messages
       */
      struct error_category : std::error_category
      {
         /**
          * The name of the vkn object the error appeared from.
          */
         [[nodiscard]] auto name() const noexcept -> const char* override;
         /**
          * Get the message associated with a specific error code.
          */
         [[nodiscard]] auto message(int err) const -> std::string override;
      };

      inline static const error_category m_category{};

   public:
      enum struct error
      {
         failed_to_create_semaphore
      };

      struct create_info
      {
         vk::Device device;
         vk::UniqueSemaphore semaphore;
      };

   public:
      semaphore() = default;
      semaphore(create_info&& info) noexcept;

      [[nodiscard]] auto value() const noexcept -> vk::Semaphore;
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::UniqueSemaphore m_semaphore{nullptr};
      vk::Device m_device{nullptr};

   private:
      /**
       * Turn an error flag and a standard error code into a vkn::error
       */
      inline static auto make_error(error err, std::error_code ec) -> vkn::error
      {
         return {{static_cast<int>(err), m_category}, static_cast<vk::Result>(ec.value())};
      }

   public:
      class builder
      {
      public:
         builder(const vkn::device& device, util::logger* p_logger) noexcept;

         [[nodiscard]] auto build() const noexcept -> vkn::result<semaphore>;

      private:
         util::logger* const mp_logger;

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
   struct is_error_code_enum<vkn::semaphore::error> : true_type
   {
   };
} // namespace std
