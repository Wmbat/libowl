#include <vkn/descriptor_pool.hpp>

#include <monads/try.hpp>

namespace vkn
{
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
   auto make_error(descriptor_pool_error err, std::error_code ec) -> vkn::error
   {
      return {{static_cast<int>(err), descriptor_pool_error_category},
              static_cast<vk::Result>(ec.value())};
   }

   auto descriptor_pool::device() const noexcept -> vk::Device { return m_value.getOwner(); }

   auto descriptor_pool::sets() const noexcept -> util::dynamic_array<vk::DescriptorSet>
   {
      return m_sets;
   }

   using builder = descriptor_pool::builder;

   builder::builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) noexcept :
      m_device{device.value()}, mp_logger{std::move(p_logger)}
   {}

   auto builder::build() -> vkn::result<descriptor_pool>
   {
      return create_descriptor_pool()
         .and_then([&](auto handle) {
            return allocate_descriptor_sets(std::move(handle));
         })
         .map([&](creation_info info) {
            descriptor_pool ret{};
            ret.m_value = std::move(info.pool);
            ret.m_sets = info.sets;
            ret.mp_logger = mp_logger;

            return ret;
         });
   }

   auto builder::set_max_sets(util::count32_t count) noexcept -> builder&
   {
      m_info.max_set_count = count;
      return *this;
   }

   auto
   builder::set_descriptor_set_layouts(const util::dynamic_array<vk::DescriptorSetLayout>& layouts)
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
      m_info.pool_sizes.emplace_back(
         vk::DescriptorPoolSize{.type = type, .descriptorCount = count.value()});
      return *this;
   }

   auto builder::create_descriptor_pool() const noexcept -> vkn::result<vk::UniqueDescriptorPool>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_device.createDescriptorPoolUnique(
                   {.maxSets = m_info.max_set_count.value(),
                    .poolSizeCount = static_cast<std::uint32_t>(std::size(m_info.pool_sizes)),
                    .pPoolSizes = std::data(m_info.pool_sizes)});
             })
         .map_error([](const vk::SystemError& e) {
            return make_error(descriptor_pool_error::failed_to_create_descriptor_pool, e.code());
         })
         .map([&](auto&& handle) {
            util::log_info(mp_logger, "[vkn] descriptor pool created");

            return std::move(handle);
         });
   }
   auto builder::allocate_descriptor_sets(vk::UniqueDescriptorPool&& handle) const
      -> vkn::result<creation_info>
   {
      util::dynamic_array<vk::DescriptorSetLayout> layouts{};
      if (m_info.singular_layout)
      {
         layouts = util::dynamic_array<vk::DescriptorSetLayout>{m_info.max_set_count.value(),
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
            monad::make_error(make_error(
               descriptor_pool_error::invalid_number_of_descriptor_set_layouts_provided, {}));
         }
      }

      return try_wrap([&] {
                return m_device.allocateDescriptorSets(
                   {.descriptorPool = handle.get(),
                    .descriptorSetCount = m_info.max_set_count.value(),
                    .pSetLayouts = layouts.data()});
             })
         .map([&](auto&& data) {
            util::log_info(mp_logger, "[vkn] {} descriptor sets created", std::size(data));

            return creation_info{.pool = std::move(handle), .sets = {data.begin(), data.end()}};
         })
         .map_error([](const vk::SystemError& err) {
            return make_error(descriptor_pool_error::failed_to_allocate_descriptor_sets,
                              err.code());
         });
   }
} // namespace vkn
