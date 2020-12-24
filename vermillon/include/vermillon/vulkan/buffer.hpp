#pragma once

#include <vermillon/vulkan/device.hpp>

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
         builder(const vkn::device& device, cacao::logger_wrapper logger) noexcept;

         [[nodiscard]] auto build() noexcept -> util::result<buffer>;

         auto set_size(std::size_t size) noexcept -> builder&;
         auto set_usage(const vk::BufferUsageFlags& flags) noexcept -> builder&;
         auto set_concurrent() noexcept -> builder&;
         auto set_desired_memory_type(const vk::MemoryPropertyFlags& flags) noexcept -> builder&;
         auto add_fallback_memory_type(const vk::MemoryPropertyFlags& flags) noexcept -> builder&;

      private:
         [[nodiscard]] auto create_buffer() const -> util::result<vk::UniqueBuffer>;
         [[nodiscard]] auto allocate_memory(vk::Buffer buffer) const
            -> util::result<vk::UniqueDeviceMemory>;

         [[nodiscard]] auto
         find_memory_requirements(std::uint32_t type_filter,
                                  const vk::MemoryPropertyFlags& properties) const noexcept
            -> monad::maybe<std::uint32_t>;

      private:
         cacao::logger_wrapper m_logger;

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
   auto to_err_code(buffer_error err) -> util::error_t;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::buffer_error> : true_type
   {
   };
} // namespace std
