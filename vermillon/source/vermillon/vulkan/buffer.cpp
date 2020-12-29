#include <vermillon/vulkan/buffer.hpp>

#include <vermillon/vulkan/core.hpp>

#include <monads/try.hpp>

namespace cacao::vulkan
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
         return to_string(static_cast<buffer_error>(err));
      }
   };

   inline static const buffer_error_category buffer_category{};

   auto to_string(buffer_error err) -> std::string
   {
      switch (err)
      {
         case buffer_error::failed_to_find_desired_memory_type:
            return "failed_to_find_desired_memory_type";
         default:
            return "UNKNOWN";
      }
   };

   auto to_err_code(buffer_error err) -> cacao::error_t
   {
      return {{static_cast<int>(err), buffer_category}};
   }

   buffer::buffer(buffer_create_info&& info)
   {
      auto logical = info.device.logical();

      m_buffer = create_buffer(logical, info);
      m_memory = allocate_memory(logical, info);

      logical.bindBufferMemory(m_buffer.get(), m_memory.get(), 0);

      info.logger.info("Buffer created");
   }

   auto buffer::get() const noexcept -> vk::Buffer { return m_buffer.get(); }
   auto buffer::memory() const noexcept -> vk::DeviceMemory { return m_memory.get(); }

   auto buffer::create_buffer(vk::Device logical, const buffer_create_info& info) const
      -> vk::UniqueBuffer
   {
      return logical.createBufferUnique(vk::BufferCreateInfo{}
                                           .setSize(info.buffer_size.value())
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
                                                .setMemoryTypeIndex(desired.value()));
      }

      if (fallback)
      {
         return logical.allocateMemoryUnique(vk::MemoryAllocateInfo{}
                                                .setAllocationSize(requirements.size)
                                                .setMemoryTypeIndex(fallback.value()));
      }

      throw cacao::runtime_error{to_err_code(buffer_error::failed_to_find_desired_memory_type)};
   }

   auto buffer::find_memory_requirements(vk::PhysicalDevice physical, std::uint32_t type_filter,
                                         const vk::MemoryPropertyFlags& properties) const noexcept
      -> monad::maybe<std::uint32_t>
   {
      const auto mem_properties = physical.getMemoryProperties();

      for (std::uint32_t i = 0; const auto& heap_type : mem_properties.memoryTypes)
      {
         if ((type_filter & (1U << i)) && (heap_type.propertyFlags & properties) == properties)
         {
            return i;
         }

         ++i;
      }

      return monad::none;
   }
} // namespace cacao::vulkan
