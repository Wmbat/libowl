#include <vermillon/vulkan/command_pool.hpp>

#include <monads/try.hpp>

namespace vkn
{
   auto command_pool::device() const noexcept -> vk::Device { return m_value.getOwner(); }
   auto command_pool::primary_cmd_buffers() const -> const crl::dynamic_array<vk::CommandBuffer>&
   {
      return m_primary_buffers;
   }
   auto command_pool::secondary_cmd_buffers() const -> const crl::dynamic_array<vk::CommandBuffer>&
   {
      return m_secondary_buffers;
   }

   auto command_pool::create_primary_buffer() const noexcept
      -> util::result<vk::UniqueCommandBuffer>
   {
      using err_t = command_pool_error;

      return monad::try_wrap<vk::SystemError>([&] {
                return device().allocateCommandBuffersUnique(
                   vk::CommandBufferAllocateInfo{}
                      .setCommandPool(value())
                      .setLevel(vk::CommandBufferLevel::ePrimary)
                      .setCommandBufferCount(1));
             })
         .map([&](auto&& buffers) {
            return std::move(buffers[0]);
         })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(err_t::failed_to_allocate_primary_command_buffers);
         });
   }

   using builder = command_pool::builder;

   builder::builder(const vkn::device& device, cacao::logger_wrapper logger) : m_logger{logger}
   {
      m_info.device = device.logical();
      m_info.queue_family_index = device.get_queue_index(queue_type::graphics);
   }

   auto builder::build() noexcept -> util::result<command_pool>
   {
      using err_t = command_pool_error;

      // clang-format off
      const auto create_info = vk::CommandPoolCreateInfo{}
         .setPNext(nullptr)
         .setQueueFamilyIndex(m_info.queue_family_index);

      return monad::try_wrap<vk::SystemError>([&] {
         return m_info.device.createCommandPoolUnique(create_info);
      }).map_error([]([[maybe_unused]] auto err) {
         return to_err_code(err_t::failed_to_create_command_pool); 
      }).and_then([&](auto handle){           
         m_logger.info("[vulkan] command pool created");

         return create_command_pool(std::move(handle)); 
      });
      // clang-format on
   }

   auto builder::set_queue_family_index(uint32_t index) noexcept -> builder&
   {
      m_info.queue_family_index = index;
      return *this;
   }
   auto builder::set_primary_buffer_count(uint32_t count) noexcept -> builder&
   {
      m_info.primary_buffer_count = count;
      return *this;
   }
   auto builder::set_secondary_buffer_count(uint32_t count) noexcept -> builder&
   {
      m_info.secondary_buffer_count = count;
      return *this;
   }

   auto builder::create_command_pool(vk::UniqueCommandPool handle) -> util::result<command_pool>
   {
      const auto primary_res = create_primary_buffers(handle.get());
      if (!primary_res.is_value())
      {
         return monad::err(primary_res.error().value());
      }

      const auto secondary_res = create_secondary_buffers(handle.get());
      if (!secondary_res.is_value())
      {
         return monad::err(secondary_res.error().value());
      }

      command_pool pool{};
      pool.m_value = std::move(handle);
      pool.m_logger = m_logger;
      pool.m_queue_index = m_info.queue_family_index;
      pool.m_primary_buffers = primary_res.value().value();
      pool.m_secondary_buffers = secondary_res.value().value();

      return pool;
   }
   auto builder::create_primary_buffers(vk::CommandPool handle)
      -> util::result<crl::dynamic_array<vk::CommandBuffer>>
   {
      using err_t = command_pool_error;

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.allocateCommandBuffers(
                   vk::CommandBufferAllocateInfo{}
                      .setCommandPool(handle)
                      .setLevel(vk::CommandBufferLevel::ePrimary)
                      .setCommandBufferCount(m_info.primary_buffer_count));
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(err_t::failed_to_allocate_primary_command_buffers);
         })
         .map([&](const auto& buffers) {
            m_logger.info("[vulkan] {0} primary command buffers created",
                          m_info.primary_buffer_count);

            return crl::dynamic_array<vk::CommandBuffer>{buffers.begin(), buffers.end()};
         });
   }
   auto builder::create_secondary_buffers(vk::CommandPool handle)
      -> util::result<crl::dynamic_array<vk::CommandBuffer>>
   {
      using err_t = command_pool_error;

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.allocateCommandBuffers(
                   vk::CommandBufferAllocateInfo{}
                      .setPNext(nullptr)
                      .setCommandPool(handle)
                      .setLevel(vk::CommandBufferLevel::eSecondary)
                      .setCommandBufferCount(m_info.primary_buffer_count));
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(err_t::failed_to_allocate_primary_command_buffers);
         })
         .map([&](const auto& buffers) {
            m_logger.info("[vkn] {0} secondary command buffers created",
                          m_info.secondary_buffer_count);

            return crl::dynamic_array<vk::CommandBuffer>{buffers.begin(), buffers.end()};
         });
   }

   struct error_category : std::error_category
   {
      /**
       * The name of the vkn object the error appeared from.
       */
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "vkn_command_pool";
      }
      /**
       * Get the message associated with a specific error code.
       */
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<command_pool_error>(err));
      }
   };

   inline static const error_category command_pool_category{};

   auto to_string(command_pool_error err) -> std::string
   {
      switch (err)
      {
         case command_pool_error::failed_to_create_command_pool:
            return "failed_to_create_command_pool";
         case command_pool_error::failed_to_allocate_primary_command_buffers:
            return "failed_to_allocate_primary_command_buffers";
         case command_pool_error::failed_to_allocate_secondary_command_buffers:
            return "failed_to_allocate_secondary_command_buffers";
         default:
            return "UNKNOWN";
      }
   };

   auto to_err_code(command_pool_error err) -> util::error_t
   {
      return {{static_cast<int>(err), command_pool_category}};
   }

} // namespace vkn
