#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

namespace vkn
{
   /**
    * The possible errors that may occur the construction of the
    * fence object
    */
   enum struct fence_error
   {
      failed_to_create_fence
   };

   /**
    * Convert an fence_error enum to a string
    */
   auto to_string(fence_error err) -> std::string;
   auto to_err_code(fence_error err) -> util::error_t;

   /**
    * A class to wrap around the vulkan fence handle. May only
    * be built using the inner builder class
    */
   class fence final : public owning_handle<vk::Fence>
   {
   public:
      /**
       * Get the device used to create the underlying handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   public:
      /**
       * Helper class to simplify the building of a fence object
       */
      class builder
      {
      public:
         builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger);

         /**
          * Attempt to create the fence object. Returns an error
          * otherwise
          */
         [[nodiscard]] auto build() const noexcept -> util::result<fence>;

         auto set_signaled(bool signaled = true) noexcept -> builder&;

      private:
         std::shared_ptr<util::logger> mp_logger{nullptr};

         struct info
         {
            vk::Device device;

            bool signaled{false};
         } m_info;
      };
   };

   class fence_observer final
   {
   public:
      using value_type = vk::Fence;
      using pointer = vk::Fence*;
      using const_pointer = const vk::Fence*;

      fence_observer() = default;
      fence_observer(value_type value) noexcept : m_value{value} {}
      fence_observer(const fence& fence) noexcept : m_value{fence.value()} {}

      auto operator->() noexcept -> pointer { return &m_value; }
      auto operator->() const noexcept -> const_pointer { return &m_value; }

      auto operator*() const noexcept -> value_type { return value(); }

      operator bool() const noexcept { return m_value; }

      [[nodiscard]] auto value() const noexcept -> value_type { return m_value; }

   private:
      value_type m_value{nullptr};
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::fence_error> : true_type
   {
   };
} // namespace std
