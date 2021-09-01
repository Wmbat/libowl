#include <libcacao/buffer.hpp>

#include <libreglisse/try.hpp>

#include <magic_enum.hpp>

using namespace reglisse;

namespace cacao
{
   struct buffer_error_category : std::error_category
   {
      /**
       * The name of the vkn object the error appeared from.
       */
      [[nodiscard]] auto name() const noexcept -> const char* override { return "vk_command_pool"; }
      /**
       * Get the message associated with a specific error code.
       */
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return std::string(magic_enum::enum_name(static_cast<buffer_error>(err)));
      }
   };

   inline static const buffer_error_category buffer_category{};

   auto make_error_condition(buffer_error code) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(code), buffer_category});
   }

   buffer::buffer(const buffer_create_info& info) :
      m_buffer(create_buffer(info.device.logical(), info)),
      m_memory(allocate_memory(info.device.logical(), info)),
      m_logger(info.logger)
   {
      auto logical = info.device.logical();

      logical.bindBufferMemory(m_buffer.get(), m_memory.get(), 0);

      m_logger.debug("Buffer created with usage: {}", vk::to_string(info.usage));
   }
   buffer::buffer(buffer_create_info&& info) :
      m_buffer(create_buffer(info.device.logical(), info)),
      m_memory(allocate_memory(info.device.logical(), info)),
      m_logger(info.logger)
   {
      auto logical = info.device.logical();

      logical.bindBufferMemory(m_buffer.get(), m_memory.get(), 0);

      m_logger.debug("Buffer created with usage: {}", vk::to_string(info.usage));
   }

   auto buffer::value() const noexcept -> vk::Buffer { return m_buffer.get(); }
   auto buffer::memory() const noexcept -> vk::DeviceMemory { return m_memory.get(); }

   auto buffer::create_buffer(vk::Device logical, const buffer_create_info& info) const
      -> vk::UniqueBuffer
   {
      return logical.createBufferUnique(vk::BufferCreateInfo{}
                                           .setSize(info.buffer_size)
                                           .setUsage(info.usage)
                                           .setSharingMode(info.set_concurrent
                                                              ? vk::SharingMode::eConcurrent
                                                              : vk::SharingMode::eExclusive));
   }

   auto buffer::allocate_memory(vk::Device logical, const buffer_create_info& info) const
      -> vk::UniqueDeviceMemory
   {
      const auto requirements = logical.getBufferMemoryRequirements(m_buffer.get());
      const auto desired = find_memory_requirements(
         info.device.physical(), requirements.memoryTypeBits, info.desired_mem_flags);
      const auto fallback = find_memory_requirements(
         info.device.physical(), requirements.memoryTypeBits, info.fallback_mem_flags);

      if (desired)
      {
         return logical.allocateMemoryUnique(vk::MemoryAllocateInfo{}
                                                .setAllocationSize(requirements.size)
                                                .setMemoryTypeIndex(desired.borrow()));
      }

      if (fallback)
      {
         return logical.allocateMemoryUnique(vk::MemoryAllocateInfo{}
                                                .setAllocationSize(requirements.size)
                                                .setMemoryTypeIndex(fallback.borrow()));
      }

      throw runtime_error{make_error_condition(buffer_error::failed_to_find_desired_memory_type)};
   }

   auto buffer::find_memory_requirements(vk::PhysicalDevice physical, std::uint32_t type_filter,
                                         const vk::MemoryPropertyFlags& properties) const noexcept
      -> reglisse::maybe<std::uint32_t>
   {
      const auto mem_properties = physical.getMemoryProperties();

      for (std::uint32_t i = 0; const auto& heap_type : mem_properties.memoryTypes)
      {
         if ((type_filter & (1U << i)) && (heap_type.propertyFlags & properties) == properties)
         {
            return some(i);
         }

         ++i;
      }

      return none;
   }
} // namespace cacao
