#pragma once

#include <vkn/device.hpp>

#include <monads/maybe.hpp>

namespace vkn
{
   /**
    * The possible errors that may occur the construction of the
    * buffer object
    */
   enum class buffer_error
   {
      failed_to_create_buffer,
      failed_to_allocate_memory,
      failed_to_find_desired_memory_type
   };

   class buffer : public owning_handle<vk::Buffer>
   {
   public:
      [[nodiscard]] auto memory() const noexcept -> vk::DeviceMemory;
      /**
       * Get the device used to create the underlying handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::UniqueDeviceMemory m_memory;

   public:
      class builder
      {
      public:
         builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) noexcept;

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
         std::shared_ptr<util::logger> mp_logger;

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
   };

   /**
    * Convert an buffer_error enum to a string
    */
   auto to_string(buffer_error err) -> std::string;
   /**
    * Convert an buffer_error enum value and an error code from a vulkan error into
    * a vkn::error
    */
} // namespace vkn
