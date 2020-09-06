#include <vkn/command_pool.hpp>

#include <monads/try.hpp>

namespace vkn
{
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
   auto make_error(command_pool_error err, std::error_code ec) -> vkn::error
   {
      return vkn::error{{static_cast<int>(err), command_pool_category},
                        static_cast<vk::Result>(ec.value())};
   }

   auto command_pool::device() const noexcept -> vk::Device { return m_value.getOwner(); }
   auto command_pool::primary_cmd_buffers() const -> const util::dynamic_array<vk::CommandBuffer>&
   {
      return m_primary_buffers;
   }
   auto command_pool::secondary_cmd_buffers() const -> const util::dynamic_array<vk::CommandBuffer>&
   {
      return m_secondary_buffers;
   }

   auto command_pool::create_primary_buffer() const noexcept -> vkn::result<vk::UniqueCommandBuffer>
   {
      using err_t = command_pool_error;

      return monad::try_wrap<vk::SystemError>([&] {
                return device().allocateCommandBuffersUnique(
                   {.commandPool = value(),
                    .level = vk::CommandBufferLevel::ePrimary,
                    .commandBufferCount = 1});
             })
         .map([&](auto&& buffers) {
            return std::move(buffers[0]);
         })
         .map_error([](auto err) {
            return make_error(err_t::failed_to_allocate_primary_command_buffers, err.code());
         });
   }

   using builder = command_pool::builder;

   builder::builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) :
      mp_logger{std::move(p_logger)}
   {
      m_info.device = device.value();
      m_info.queue_family_index = device.get_queue_index(queue::type::graphics);
   }

   auto builder::build() noexcept -> vkn::result<command_pool>
   {
      using err_t = command_pool_error;

      // clang-format off
      const auto create_info = vk::CommandPoolCreateInfo{}
         .setPNext(nullptr)
         .setFlags({})
         .setQueueFamilyIndex(m_info.queue_family_index);

      return monad::try_wrap<vk::SystemError>([&] {
         return m_info.device.createCommandPoolUnique(create_info);
      }).map_error([](auto err) {
         return make_error(err_t::failed_to_create_command_pool, err.code()); 
      }).and_then([&](auto handle){           
         util::log_info(mp_logger, "[vkn] command pool created");

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

   auto builder::create_command_pool(vk::UniqueCommandPool handle) -> vkn::result<command_pool>
   {
      const auto primary_res = create_primary_buffers(handle.get());
      if (!primary_res.is_value())
      {
         return monad::make_error(primary_res.error().value());
      }

      const auto secondary_res = create_secondary_buffers(handle.get());
      if (!secondary_res.is_value())
      {
         return monad::make_error(secondary_res.error().value());
      }

      command_pool pool{};
      pool.m_value = std::move(handle);
      pool.mp_logger = mp_logger;
      pool.m_queue_index = m_info.queue_family_index;
      pool.m_primary_buffers = primary_res.value().value();
      pool.m_secondary_buffers = secondary_res.value().value();

      return pool;
   }
   auto builder::create_primary_buffers(vk::CommandPool handle)
      -> vkn::result<util::dynamic_array<vk::CommandBuffer>>
   {
      using err_t = command_pool_error;

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.allocateCommandBuffers(
                   vk::CommandBufferAllocateInfo{}
                      .setPNext(nullptr)
                      .setCommandPool(handle)
                      .setLevel(vk::CommandBufferLevel::ePrimary)
                      .setCommandBufferCount(m_info.primary_buffer_count));
             })
         .map_error([](auto err) {
            return make_error(err_t::failed_to_allocate_primary_command_buffers, err.code());
         })
         .map([&](const auto& buffers) {
            util::log_info(mp_logger, "[vkn] {0} primary command buffers created",
                           m_info.primary_buffer_count);

            return util::dynamic_array<vk::CommandBuffer>{buffers.begin(), buffers.end()};
         });
   }
   auto builder::create_secondary_buffers(vk::CommandPool handle)
      -> vkn::result<util::dynamic_array<vk::CommandBuffer>>
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
         .map_error([](auto err) {
            return make_error(err_t::failed_to_allocate_primary_command_buffers, err.code());
         })
         .map([&](const auto& buffers) {
            util::log_info(mp_logger, "[vkn] {0} secondary command buffers created",
                           m_info.secondary_buffer_count);

            return util::dynamic_array<vk::CommandBuffer>{buffers.begin(), buffers.end()};
         });
   }
} // namespace vkn
