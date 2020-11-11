#include <vkn/buffer.hpp>

#include <vkn/core.hpp>

#include <monads/try.hpp>

namespace vkn
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
         case buffer_error::failed_to_create_buffer:
            return "failed_to_create_buffer";
         case buffer_error::failed_to_allocate_memory:
            return "failed_to_allocate_memory";
         default:
            return "UNKNOWN";
      }
   };

   auto to_err_code(buffer_error err) -> util::error_t
   {
      return {{static_cast<int>(err), buffer_category}};
   }

   auto buffer::memory() const noexcept -> vk::DeviceMemory { return m_memory.get(); }
   auto buffer::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   using builder = buffer::builder;

   builder::builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) noexcept :
      mp_logger{std::move(p_logger)}
   {
      m_info.device = device.logical_device();
      m_info.physical_device = device.physical_device();
   }

   auto builder::build() const noexcept -> util::result<buffer>
   {
      const auto allocate_n_construct = [&](vk::UniqueBuffer buffer) noexcept {
         return allocate_memory(buffer.get()).map([&](vk::UniqueDeviceMemory memory) noexcept {
            util::log_info(mp_logger, "[vkn] buffer of size {} created", m_info.size);

            m_info.device.bindBufferMemory(buffer.get(), memory.get(), 0);

            class buffer b;
            b.m_value = std::move(buffer);
            b.m_memory = std::move(memory);

            return b;
         });
      };

      return create_buffer().and_then(allocate_n_construct);
   }

   auto builder::set_size(std::size_t size) noexcept -> builder&
   {
      m_info.size = size;
      return *this;
   }
   auto builder::set_usage(const vk::BufferUsageFlags& flags) noexcept -> builder&
   {
      m_info.flags = flags;
      return *this;
   }
   auto builder::set_concurrent() noexcept -> builder&
   {
      m_info.mode = vk::SharingMode::eConcurrent;
      return *this;
   }
   auto builder::set_desired_memory_type(const vk::MemoryPropertyFlags& flags) noexcept -> builder&
   {
      m_info.desired_mem_flags = flags;
      return *this;
   }
   auto builder::add_fallback_memory_type(const vk::MemoryPropertyFlags& flags) noexcept -> builder&
   {
      m_info.fallback_mem_flags = flags;
      return *this;
   }

   auto builder::create_buffer() const -> util::result<vk::UniqueBuffer>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createBufferUnique(
                   {.size = m_info.size, .usage = m_info.flags, .sharingMode = m_info.mode});
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(buffer_error::failed_to_create_buffer);
         });
   }

   auto builder::allocate_memory(vk::Buffer buffer) const -> util::result<vk::UniqueDeviceMemory>
   {
      const auto requirements = m_info.device.getBufferMemoryRequirements(buffer);
      auto error_res = util::result<vk::UniqueDeviceMemory>{
         monad::err(to_err_code(buffer_error::failed_to_find_desired_memory_type))};

      const auto alloc_memory = [&](uint32_t index) noexcept {
         return monad::try_wrap<vk::SystemError>([&] {
                   return m_info.device.allocateMemoryUnique(
                      {.allocationSize = requirements.size, .memoryTypeIndex = index});
                })
            .map_error([]([[maybe_unused]] auto err) {
               return to_err_code(buffer_error::failed_to_allocate_memory);
            });
      };

      return find_memory_requirements(requirements.memoryTypeBits, m_info.desired_mem_flags)
         .map_or(alloc_memory,
                 find_memory_requirements(requirements.memoryTypeBits, m_info.fallback_mem_flags)
                    .map_or(alloc_memory, std::move(error_res)));
   }
   auto builder::find_memory_requirements(std::uint32_t type_filter,
                                          const vk::MemoryPropertyFlags& properties) const noexcept
      -> monad::maybe<std::uint32_t>
   {
      const auto mem_properties = m_info.physical_device.getMemoryProperties();

      for (std::uint32_t i = 0; const auto& heap_type : mem_properties.memoryTypes)
      {
         if ((type_filter & (1U << i)) && (heap_type.propertyFlags == properties))
         {
            return i;
         }

         ++i;
      }

      return monad::none;
   }
} // namespace vkn
