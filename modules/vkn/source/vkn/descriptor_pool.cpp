#include <vkn/descriptor_pool.hpp>

#include <vkn/framebuffer.hpp>

#include <monads/try.hpp>

namespace vkn
{
   auto descriptor_pool::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   auto descriptor_pool::sets() const noexcept -> crl::dynamic_array<vk::DescriptorSet>
   {
      return m_sets;
   }

   using builder = descriptor_pool::builder;

   builder::builder(const vkn::device& device, util::logger_wrapper logger) noexcept :
      m_device{device.logical()}, m_logger{logger}
   {}

   auto builder::build() -> util::result<descriptor_pool>
   {
      return create_descriptor_pool()
         .and_then([&](auto handle) {
            return allocate_descriptor_sets(std::move(handle));
         })
         .map([&](creation_info info) {
            descriptor_pool ret{};
            ret.m_value = std::move(info.pool);
            ret.m_sets = info.sets;
            ret.m_logger = m_logger;

            return ret;
         });
   }

   auto builder::set_max_sets(util::count32_t count) noexcept -> builder&
   {
      m_info.max_set_count = count;
      return *this;
   }

   auto
   builder::set_descriptor_set_layouts(const crl::dynamic_array<vk::DescriptorSetLayout>& layouts)
      -> builder&
   {
      m_info.unique_layouts = layouts;
      return *this;
   }
   auto builder::set_descriptor_set_layout(vk::DescriptorSetLayout layout) noexcept -> builder&
   {
      m_info.singular_layout = layout;
      return *this;
   }

   auto builder::add_pool_size(vk::DescriptorType type, util::count32_t count) -> builder&
   {
      m_info.pool_sizes.append(
         vk::DescriptorPoolSize{.type = type, .descriptorCount = count.value()});
      return *this;
   }

   auto builder::create_descriptor_pool() noexcept -> util::result<vk::UniqueDescriptorPool>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_device.createDescriptorPoolUnique(
                   {.maxSets = m_info.max_set_count.value(),
                    .poolSizeCount = static_cast<std::uint32_t>(std::size(m_info.pool_sizes)),
                    .pPoolSizes = std::data(m_info.pool_sizes)});
             })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(descriptor_pool_error::failed_to_create_descriptor_pool);
         })
         .map([&](vk::UniqueDescriptorPool&& handle) {
            m_logger.info("[vulkan] descriptor pool created");

            return std::move(handle);
         });
   }
   auto builder::allocate_descriptor_sets(vk::UniqueDescriptorPool&& handle)
      -> util::result<creation_info>
   {
      crl::dynamic_array<vk::DescriptorSetLayout> layouts{};
      if (m_info.singular_layout)
      {
         layouts = crl::dynamic_array<vk::DescriptorSetLayout>{m_info.max_set_count.value(),
                                                               m_info.singular_layout};
      }
      else
      {
         layouts = m_info.unique_layouts;
      }

      if (!m_info.singular_layout)
      {
         if (std::size(m_info.unique_layouts) != m_info.max_set_count.value())
         {
            monad::err(to_err_code(
               descriptor_pool_error::invalid_number_of_descriptor_set_layouts_provided));
         }
      }

      return try_wrap([&] {
                return m_device.allocateDescriptorSets(
                   {.descriptorPool = handle.get(),
                    .descriptorSetCount = m_info.max_set_count.value(),
                    .pSetLayouts = layouts.data()});
             })
         .map([&](auto&& data) {
            m_logger.info("[vulkan] {} descriptor sets created", std::size(data));

            return creation_info{.pool = std::move(handle), .sets = {data.begin(), data.end()}};
         })
         .map_error([]([[maybe_unused]] auto err) {
            return to_err_code(descriptor_pool_error::failed_to_allocate_descriptor_sets);
         });
   }

   struct descriptor_pool_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "vkn_descriptor_pool";
      }

      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<descriptor_pool_error>(err));
      }
   };

   inline static const descriptor_pool_error_category descriptor_pool_error_category{};

   auto to_string(descriptor_pool_error err) -> std::string
   {
      switch (err)
      {
         case descriptor_pool_error::failed_to_create_descriptor_pool:
            return "failed_to_create_descriptor_pool";
         case descriptor_pool_error::failed_to_allocate_descriptor_sets:
            return "failed_to_allocate_descriptor_sets";
         case descriptor_pool_error::invalid_number_of_descriptor_set_layouts_provided:
            return "invalid_number_of_descriptor_set_layouts_provided";
         default:
            return "UNKNOWN";
      }
   }
   auto to_err_code(descriptor_pool_error err) -> util::error_t
   {
      return {{static_cast<int>(err), descriptor_pool_error_category}};
   }

} // namespace vkn
