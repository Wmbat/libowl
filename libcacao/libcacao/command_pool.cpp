#include <libcacao/command_pool.hpp>

#include <libreglisse/try.hpp>

#include <vulkan/vulkan_structs.hpp>

using namespace reglisse;

namespace cacao
{
   class command_pool_error_category : public std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "cacao_command_pool";
      }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return std::string(magic_enum::enum_name(static_cast<command_pool_error>(err)));
      }
   };

   inline static const command_pool_error_category command_pool_category;

   auto make_error_condition(command_pool_error code) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(code), command_pool_category});
   }

   command_pool::command_pool(const command_pool_create_info& info) :
      m_queue_index(info.queue_family_index
                       ? info.queue_family_index.borrow()
                       : info.device.get_queue_index(queue_flag_bits::graphics)),
      m_pool(info.device.logical().createCommandPoolUnique({.queueFamilyIndex = m_queue_index})),
      m_primary_buffers(info.device.logical().allocateCommandBuffers(
         {.commandPool = m_pool.get(),
          .level = vk::CommandBufferLevel::ePrimary,
          .commandBufferCount = info.primary_buffer_count})),
      m_secondary_buffers(info.device.logical().allocateCommandBuffers(
         {.commandPool = m_pool.get(),
          .level = vk::CommandBufferLevel::eSecondary,
          .commandBufferCount = info.secondary_buffer_count})),
      m_logger(info.logger)
   {
      m_logger.debug(
         "Command pool created on queue family {} with {} primary buffers and {} secondary buffers",
         m_queue_index, info.primary_buffer_count, info.secondary_buffer_count);
   }
   command_pool::command_pool(command_pool_create_info&& info) :
      m_queue_index(info.queue_family_index
                       ? info.queue_family_index.borrow()
                       : info.device.get_queue_index(queue_flag_bits::graphics)),
      m_pool(info.device.logical().createCommandPoolUnique({.queueFamilyIndex = m_queue_index})),
      m_primary_buffers(info.device.logical().allocateCommandBuffers(
         {.commandPool = m_pool.get(),
          .level = vk::CommandBufferLevel::ePrimary,
          .commandBufferCount = info.primary_buffer_count})),
      m_secondary_buffers(info.device.logical().allocateCommandBuffers(
         {.commandPool = m_pool.get(),
          .level = vk::CommandBufferLevel::eSecondary,
          .commandBufferCount = info.secondary_buffer_count})),
      m_logger(info.logger)
   {
      m_logger.debug(
         "Command pool created on queue family {} with {} primary buffers and {} secondary buffers",
         m_queue_index, info.primary_buffer_count, info.secondary_buffer_count);
   }

   auto to_vk_level(command_buffer_level level)
   {
      return level == command_buffer_level::primary ? vk::CommandBufferLevel::ePrimary
                                                    : vk::CommandBufferLevel::eSecondary;
   }

   auto LIBCACAO_SYMEXPORT create_standalone_command_buffers(const device& device,
                                                             const command_pool& pool,
                                                             command_buffer_level level,
                                                             mannele::u32 count)
      -> std::vector<vk::UniqueCommandBuffer>
   {
      return device.logical().allocateCommandBuffersUnique(
         {.commandPool = pool.value(), .level = to_vk_level(level), .commandBufferCount = count});
   }
} // namespace cacao
