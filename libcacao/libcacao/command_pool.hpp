#ifndef LIBCACAO_COMMAND_POOL_HPP
#define LIBCACAO_COMMAND_POOL_HPP

#include <libcacao/device.hpp>
#include <libcacao/export.hpp>

#include <libmannele/logging/log_ptr.hpp>

#include <libreglisse/maybe.hpp>

namespace cacao
{
   enum class command_pool_error
   {
      failed_to_create_command_pool,
      failed_to_allocate_primary_command_buffers,
      failed_to_allocate_secondary_command_buffers
   };

   auto LIBCACAO_SYMEXPORT make_error_condition(command_pool_error code) -> std::error_condition;

   enum class command_buffer_level
   {
      primary,
      secondary
   };

   struct LIBCACAO_SYMEXPORT command_pool_create_info
   {
      const cacao::device& device;

      reglisse::maybe<mannele::u32> queue_family_index = reglisse::none;

      mannele::u32 primary_buffer_count = 0;
      mannele::u32 secondary_buffer_count = 0;

      mannele::log_ptr logger = nullptr;
   };

   class LIBCACAO_SYMEXPORT command_pool
   {
   public:
      command_pool() = default;
      command_pool(const command_pool_create_info& info);
      command_pool(command_pool_create_info&& info);

      [[nodiscard]] auto value() const noexcept -> vk::CommandPool;
      [[nodiscard]] auto primary_buffers() const noexcept -> std::span<const vk::CommandBuffer>;
      [[nodiscard]] auto secondary_buffers() const noexcept -> std::span<const vk::CommandBuffer>;

   private:
      mannele::u32 m_queue_index = std::numeric_limits<mannele::u32>::max();

      vk::UniqueCommandPool m_pool;
      std::vector<vk::CommandBuffer> m_primary_buffers;
      std::vector<vk::CommandBuffer> m_secondary_buffers;

      mannele::log_ptr m_logger = nullptr;
   };

   auto LIBCACAO_SYMEXPORT create_standalone_command_buffers(const device& device,
                                                             const command_pool& pool,
                                                             command_buffer_level level,
                                                             mannele::u32 count)
      -> std::vector<vk::UniqueCommandBuffer>;
} // namespace cacao

#endif // LIBCACAO_COMMAND_POOL_HPP
