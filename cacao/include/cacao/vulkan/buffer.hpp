#pragma once

#include <cacao/device.hpp>

#include <monads/maybe.hpp>

namespace cacao::vulkan
{
   /**
    * The possible errors that may occur the construction of the
    * buffer object
    */
   enum class buffer_error
   {
      failed_to_find_desired_memory_type
   };

   struct buffer_create_info
   {
      const cacao::device& device;

      cacao::size_t buffer_size{};

      vk::BufferUsageFlags usage{};
      vk::MemoryPropertyFlags desired_mem_flags{};
      vk::MemoryPropertyFlags fallback_mem_flags{};

      bool set_concurrent{false};

      util::logger_wrapper logger;
   };

   class buffer
   {
   public:
      buffer() = default;
      buffer(buffer_create_info&& info);

      [[nodiscard]] auto get() const noexcept -> vk::Buffer;
      [[nodiscard]] auto memory() const noexcept -> vk::DeviceMemory;

   private:
      auto create_buffer(vk::Device logical, const buffer_create_info& info) const
         -> vk::UniqueBuffer;
      auto allocate_memory(vk::Device logical, const buffer_create_info& info) const
         -> vk::UniqueDeviceMemory;

      auto find_memory_requirements(vk::PhysicalDevice physical, std::uint32_t type_filter,
                                    const vk::MemoryPropertyFlags& properties) const noexcept
         -> monad::maybe<std::uint32_t>;

   private:
      vk::UniqueBuffer m_buffer;
      vk::UniqueDeviceMemory m_memory;
   };

   /**
    * Convert an buffer_error enum to a string
    */
   auto to_string(buffer_error err) -> std::string;
   auto to_err_code(buffer_error err) -> cacao::error_t;
} // namespace cacao::vulkan

namespace std
{
   template <>
   struct is_error_code_enum<cacao::vulkan::buffer_error> : true_type
   {
   };
} // namespace std
