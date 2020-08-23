#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

namespace vkn
{
   /**
    * A class to wrap around the vulkan fence handle. May only
    * be built using the inner builder class
    */
   class fence final
   {
   public:
      using value_type = vk::Fence;
      using pointer = vk::Fence*;
      using const_pointer = const vk::Fence*;

      /**
       * The possible errors that may occur the construction of the
       * fence object
       */
      enum struct error
      {
         failed_to_create_fence
      };

   public:
      fence() = default;

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
      [[nodiscard]] auto value() const noexcept -> vk::Fence;
      /**
       * Get the device used to create the underlying handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::UniqueFence m_fence{nullptr};

   public:
      /**
       * Helper class to simplify the building of a fence object
       */
      class builder
      {
      public:
         builder(const vkn::device& device, util::logger* p_logger);

         /**
          * Attempt to create the fence object. Returns an error
          * otherwise
          */
         [[nodiscard]] auto build() const noexcept -> vkn::result<fence>;

         auto set_signaled(bool signaled = true) noexcept -> builder&;

      private:
         util::logger* const mp_logger{nullptr};

         struct info
         {
            vk::Device device;

            bool signaled{false};
         } m_info;
      };

   private:
      struct create_info
      {
         vk::UniqueFence fence;
      };

      fence(create_info&& info) noexcept;

      struct error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override;
         [[nodiscard]] auto message(int err) const -> std::string override;
      };

      inline static const error_category m_category{};

      /**
       * Turn an error flag and a standard error code into a vkn::error
       */
      inline static auto make_error(error err, std::error_code ec) -> vkn::error
      {
         return {{static_cast<int>(err), m_category}, static_cast<vk::Result>(ec.value())};
      }
   };

   class fence_observer final
   {
   public:
      using value_type = vk::Fence;
      using pointer = vk::Fence*;
      using const_pointer = const vk::Fence*;

      constexpr fence_observer() = default;
      constexpr fence_observer(value_type value) noexcept : m_value{value} {}
      constexpr fence_observer(const fence& fence) noexcept : m_value{vkn::value(fence)} {}

      constexpr auto operator->() noexcept -> pointer { return &m_value; }
      constexpr auto operator->() const noexcept -> const_pointer { return &m_value; }

      constexpr auto operator*() const noexcept -> value_type { return value(); }

      constexpr operator bool() const noexcept { return m_value; }

      [[nodiscard]] constexpr auto value() const noexcept -> value_type { return m_value; }

   private:
      value_type m_value{nullptr};
   };
} // namespace vkn
