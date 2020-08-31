#include "util/logger.hpp"
#include <vkn/buffer.hpp>

#include <vkn/core.hpp>

#include <monads/try.hpp>

namespace vkn
{
   namespace detail
   {
      auto to_string(buffer::error_type err) -> std::string
      {
         using error = buffer::error_type;

         switch (err)
         {
            case error::failed_to_create_buffer:
               return "failed_to_create_buffer";
            case error::failed_to_allocate_memory:
               return "failed_to_allocate_memory";
            default:
               return "UNKNOWN";
         }
      };
   } // namespace detail

   auto buffer::error_category::name() const noexcept -> const char* { return "vk_command_pool"; }
   auto buffer::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<error_type>(err));
   }

   buffer::buffer(create_info&& info) noexcept :
      m_buffer{std::move(info.buffer)}, m_memory{std::move(info.memory)}
   {}

   auto buffer::operator->() noexcept -> pointer { return &m_buffer.get(); }
   auto buffer::operator->() const noexcept -> const_pointer { return &m_buffer.get(); }

   auto buffer::operator*() const noexcept -> value_type { return value(); }

   buffer::operator bool() const noexcept { return m_buffer.get(); }

   auto buffer::value() const noexcept -> value_type { return m_buffer.get(); }
   auto buffer::memory() const noexcept -> vk::DeviceMemory { return m_memory.get(); }
   auto buffer::device() const noexcept -> vk::Device { return m_buffer.getOwner(); }

   using builder = buffer::builder;

   builder::builder(const vkn::device& device, util::logger* p_logger) noexcept :
      mp_logger{p_logger}
   {
      m_info.device = device.value();
      m_info.physical_device = device.physical().value();
   }

   auto builder::build() const noexcept -> vkn::result<buffer>
   {
      const auto allocate_n_construct = [&](vk::UniqueBuffer buffer) noexcept {
         return allocate_memory(buffer.get()).map([&](vk::UniqueDeviceMemory memory) noexcept {
            util::log_info(mp_logger, "[vkn] buffer created");

            m_info.device.bindBufferMemory(buffer.get(), memory.get(), 0);

            return vkn::buffer{{.buffer = std::move(buffer), .memory = std::move(memory)}};
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

   auto builder::create_buffer() const -> vkn::result<vk::UniqueBuffer>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createBufferUnique(
                   {.size = m_info.size, .usage = m_info.flags, .sharingMode = m_info.mode});
             })
         .map_error([](const vk::SystemError& err) {
            return make_error(error_type::failed_to_create_buffer, err.code());
         });
   }

   auto builder::allocate_memory(vk::Buffer buffer) const -> vkn::result<vk::UniqueDeviceMemory>
   {
      const auto requirements = m_info.device.getBufferMemoryRequirements(buffer);
      auto error_res = vkn::result<vk::UniqueDeviceMemory>{
         make_error_res(error_type::failed_to_find_desired_memory_type, {})};

      const auto alloc_memory = [&](uint32_t index) noexcept {
         return monad::try_wrap<vk::SystemError>([&] {
                   return m_info.device.allocateMemoryUnique(
                      {.allocationSize = requirements.size, .memoryTypeIndex = index});
                })
            .map_error([](const vk::SystemError& err) {
               return make_error(error_type::failed_to_allocate_memory, err.code());
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
         if ((type_filter & (1 << i)) && (heap_type.propertyFlags == properties))
         {
            return i;
         }

         ++i;
      }

      return monad::none;
   }
} // namespace vkn
