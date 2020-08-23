#include <vkn/command_pool.hpp>

#include <monads/try.hpp>

namespace vkn
{
   namespace detail
   {
      auto to_string(command_pool::error_type err) -> std::string
      {
         using error = command_pool::error_type;

         switch (err)
         {
            case error::failed_to_create_command_pool:
               return "failed_to_create_command_pool";
            case error::failed_to_allocate_primary_command_buffers:
               return "failed_to_allocate_primary_command_buffers";
            case error::failed_to_allocate_secondary_command_buffers:
               return "failed_to_allocate_secondary_command_buffers";
            default:
               return "UNKNOWN";
         }
      };

      auto make_error(command_pool::error_type flag, std::error_code ec) -> vkn::error
      {
         return vkn::error{command_pool::make_error_code(flag),
                           static_cast<vk::Result>(ec.value())};
      }
   } // namespace detail

   auto command_pool::error_category::name() const noexcept -> const char*
   {
      return "vk_command_pool";
   }
   auto command_pool::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<command_pool::error_type>(err));
   }

   command_pool::command_pool(create_info&& info) :
      m_command_pool{std::move(info.command_pool)}, m_queue_index{info.queue_index},
      m_primary_buffers{info.primary_buffers}, m_secondary_buffers{info.secondary_buffers}
   {}

   auto command_pool::operator->() noexcept -> pointer { return &m_command_pool.get(); }
   auto command_pool::operator->() const noexcept -> const_pointer { return &m_command_pool.get(); }

   auto command_pool::operator*() const noexcept -> value_type { return value(); }

   command_pool::operator bool() const noexcept { return m_command_pool.get(); }

   auto command_pool::value() const noexcept -> vk::CommandPool { return m_command_pool.get(); }
   auto command_pool::device() const noexcept -> vk::Device { return m_command_pool.getOwner(); }
   auto command_pool::primary_cmd_buffers() const -> const util::dynamic_array<vk::CommandBuffer>&
   {
      return m_primary_buffers;
   }
   auto command_pool::secondary_cmd_buffers() const -> const util::dynamic_array<vk::CommandBuffer>&
   {
      return m_secondary_buffers;
   }

   using builder = command_pool::builder;

   builder::builder(const vkn::device& device, util::logger* plogger) : m_plogger{plogger}
   {
      m_info.device = device.value();
      m_info.queue_family_index = device.get_queue_index(queue::type::graphics);
   }

   auto builder::build() noexcept -> vkn::result<command_pool>
   {
      using err_t = command_pool::error_type;

      // clang-format off
      const auto create_info = vk::CommandPoolCreateInfo{}
         .setPNext(nullptr)
         .setFlags({})
         .setQueueFamilyIndex(m_info.queue_family_index);

      return monad::try_wrap<vk::SystemError>([&] {
         return m_info.device.createCommandPoolUnique(create_info);
      }).left_map([](auto err) {
         return detail::make_error(err_t::failed_to_create_command_pool, err.code()); 
      }).right_flat_map([&](auto handle){           
         util::log_info(m_plogger, "[vkn] command pool created");

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
      if (!primary_res.is_right())
      {
         return monad::make_left(primary_res.left().value());
      }

      const auto secondary_res = create_secondary_buffers(handle.get());
      if (!secondary_res.is_right())
      {
         return monad::make_left(secondary_res.left().value());
      }

      // clang-format off
      return monad::make_right(command_pool{{
         .command_pool = std::move(handle),
         .queue_index = m_info.queue_family_index,
         .primary_buffers = primary_res.right().value(),
         .secondary_buffers = secondary_res.right().value()
      }});
      // clang-format on
   }
   auto builder::create_primary_buffers(vk::CommandPool handle)
      -> vkn::result<util::dynamic_array<vk::CommandBuffer>>
   {
      using err_t = command_pool::error_type;

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.allocateCommandBuffers(
                   vk::CommandBufferAllocateInfo{}
                      .setPNext(nullptr)
                      .setCommandPool(handle)
                      .setLevel(vk::CommandBufferLevel::ePrimary)
                      .setCommandBufferCount(m_info.primary_buffer_count));
             })
         .left_map([](auto err) {
            return detail::make_error(err_t::failed_to_allocate_primary_command_buffers,
                                      err.code());
         })
         .right_map([&](const auto& buffers) {
            util::log_info(m_plogger, "[vkn] {0} primary command buffers created",
                           m_info.primary_buffer_count);

            return util::dynamic_array<vk::CommandBuffer>{buffers.begin(), buffers.end()};
         });
   }
   auto builder::create_secondary_buffers(vk::CommandPool handle)
      -> vkn::result<util::dynamic_array<vk::CommandBuffer>>
   {
      using err_t = command_pool::error_type;

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.allocateCommandBuffers(
                   vk::CommandBufferAllocateInfo{}
                      .setPNext(nullptr)
                      .setCommandPool(handle)
                      .setLevel(vk::CommandBufferLevel::eSecondary)
                      .setCommandBufferCount(m_info.primary_buffer_count));
             })
         .left_map([](auto err) {
            return detail::make_error(err_t::failed_to_allocate_primary_command_buffers,
                                      err.code());
         })
         .right_map([&](const auto& buffers) {
            util::log_info(m_plogger, "[vkn] {0} secondary command buffers created",
                           m_info.secondary_buffer_count);

            return util::dynamic_array<vk::CommandBuffer>{buffers.begin(), buffers.end()};
         });
   }
} // namespace vkn
