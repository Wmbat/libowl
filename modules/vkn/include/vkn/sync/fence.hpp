#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

namespace vkn
{
   class fence final
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
         failed_to_create_fence
      };

      struct create_info
      {
         vk::Device device;
         vk::Fence fence;
      };

   public:
      fence() = default;
      fence(create_info&& info) noexcept;
      fence(const fence& other) = delete;
      fence(fence&& other) noexcept;
      ~fence();

      auto operator=(const fence&) = delete;
      auto operator=(fence&& rhs) noexcept -> fence&;

      [[nodiscard]] auto value() const noexcept -> vk::Fence;
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::Fence m_fence{nullptr};
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
         builder(const vkn::device& device, util::logger* p_logger);

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
   };
} // namespace vkn
