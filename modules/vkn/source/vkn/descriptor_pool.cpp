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

   using builder = descriptor_pool::builder;

   builder::builder(const vkn::device& device, util::logger* p_logger) noexcept :
      m_device{device.value()}, mp_logger{p_logger}
   {}

   auto builder::build() -> vkn::result<descriptor_pool>
   {
      return monad::try_wrap<vk::SystemError>([&] {
                return m_device.createDescriptorPoolUnique(
                   {.maxSets = m_info.max_set_count.value(),
                    .poolSizeCount = static_cast<std::uint32_t>(std::size(m_info.pool_sizes)),
                    .pPoolSizes = std::data(m_info.pool_sizes)});
             })
         .map([](vk::UniqueDescriptorPool pool) {
            descriptor_pool ret{};
            ret.m_value = std::move(pool);

            return ret;
         })
         .map_error([](const vk::SystemError& e) {
            return make_error(descriptor_pool_error::failed_to_create_descriptor_pool, e.code());
         });
   }

   auto builder::set_max_sets(util::count32_t count) noexcept -> builder&
   {
      m_info.max_set_count = count;
      return *this;
   }

   auto builder::add_pool_size(vk::DescriptorType type, util::count32_t count) -> builder&
   {
      m_info.pool_sizes.emplace_back(
         vk::DescriptorPoolSize{.type = type, .descriptorCount = count.value()});
      return *this;
   }
} // namespace vkn
