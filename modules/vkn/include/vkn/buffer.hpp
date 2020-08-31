#pragma once

#include <vkn/device.hpp>

#include <monads/maybe.hpp>

namespace vkn
{
   class buffer
   {
   public:
      using value_type = vk::Buffer;
      using pointer = vk::Buffer*;
      using const_pointer = const vk::Buffer*;

      enum class error_type
      {
         failed_to_create_buffer,
         failed_to_allocate_memory,
         failed_to_find_desired_memory_type
      };

   public:
      buffer() noexcept = default;

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
      [[nodiscard]] auto memory() const noexcept -> vk::DeviceMemory;
      /**
       * Get the device used to create the underlying handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::UniqueBuffer m_buffer;
      vk::UniqueDeviceMemory m_memory;

   public:
      class builder
      {
      public:
         builder(const vkn::device& device, util::logger* p_logger) noexcept;

         [[nodiscard]] auto build() const noexcept -> vkn::result<buffer>;

         auto set_size(std::size_t size) noexcept -> builder&;
         auto set_usage(const vk::BufferUsageFlags& flags) noexcept -> builder&;
         auto set_concurrent() noexcept -> builder&;
         auto set_desired_memory_type(const vk::MemoryPropertyFlags& flags) noexcept -> builder&;
         auto add_fallback_memory_type(const vk::MemoryPropertyFlags& flags) noexcept -> builder&;

      private:
         [[nodiscard]] auto create_buffer() const -> vkn::result<vk::UniqueBuffer>;
         [[nodiscard]] auto allocate_memory(vk::Buffer buffer) const
            -> vkn::result<vk::UniqueDeviceMemory>;

         [[nodiscard]] auto
         find_memory_requirements(std::uint32_t type_filter,
                                  const vk::MemoryPropertyFlags& properties) const noexcept
            -> monad::maybe<std::uint32_t>;

      private:
         util::logger* const mp_logger;

         struct info
         {
            vk::Device device;
            vk::PhysicalDevice physical_device;

            std::size_t size{};
            vk::BufferUsageFlags flags{};
            vk::SharingMode mode{vk::SharingMode::eExclusive};

            vk::MemoryPropertyFlags desired_mem_flags;
            vk::MemoryPropertyFlags fallback_mem_flags;
         } m_info;
      };

   private:
      struct create_info
      {
         vk::UniqueBuffer buffer;
         vk::UniqueDeviceMemory memory;
      };

      buffer(create_info&& info) noexcept;

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

      static auto make_error(error_type flag, std::error_code ec) -> vkn::error
      {
         return vkn::error{{static_cast<int>(flag), m_category},
                           static_cast<vk::Result>(ec.value())};
      }

      static auto make_error_res(error_type flag, std::error_code ec) -> monad::error_t<vkn::error>
      {
         return {make_error(flag, ec)};
      }
   };
} // namespace vkn
