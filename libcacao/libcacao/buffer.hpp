/**
 * @file libcacao/buffer.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief 
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_BUFFER_HPP_
#define LIBCACAO_BUFFER_HPP_

#include <libcacao/device.hpp>
#include <libcacao/export.hpp>

// Third Party Libraries

#include <libreglisse/maybe.hpp>

// C++ Standard Library

#include <system_error> // NOLINT

namespace cacao
{
   /**
    * The possible errors that may occur the construction of the
    * buffer object
    */
   enum class buffer_error
   {
      failed_to_find_desired_memory_type
   };

   auto LIBCACAO_SYMEXPORT make_error_condition(buffer_error) -> std::error_condition;

   struct buffer_create_info
   {
      const cacao::device& device;

      mannele::u64 buffer_size{};

      vk::BufferUsageFlags usage{};
      vk::MemoryPropertyFlags desired_mem_flags{};
      vk::MemoryPropertyFlags fallback_mem_flags{};

      bool set_concurrent{false};

      mannele::log_ptr logger;
   };

   class LIBCACAO_SYMEXPORT buffer
   {
   public:
      buffer() = default;
      explicit buffer(const buffer_create_info& info);
      explicit buffer(buffer_create_info&& info);

      [[nodiscard]] auto value() const noexcept -> vk::Buffer;
      [[nodiscard]] auto memory() const noexcept -> vk::DeviceMemory;

   private:
      [[nodiscard]] auto create_buffer(vk::Device logical, const buffer_create_info& info) const
         -> vk::UniqueBuffer;
      [[nodiscard]] auto allocate_memory(vk::Device logical, const buffer_create_info& info) const
         -> vk::UniqueDeviceMemory;

      [[nodiscard]] auto
      find_memory_requirements(vk::PhysicalDevice physical, std::uint32_t type_filter,
                               const vk::MemoryPropertyFlags& properties) const noexcept
         -> reglisse::maybe<std::uint32_t>;

   private:
      vk::UniqueBuffer m_buffer;
      vk::UniqueDeviceMemory m_memory;

      mannele::log_ptr m_logger;
   };
} // namespace cacao

#endif // LIBCACAO_BUFFER_HPP_
