#include <vkn/descriptor_set_layout.hpp>

#include <utility>

namespace vkn
{
   auto descriptor_set_layout::device() const -> vk::Device { return m_value.getOwner(); }
   auto descriptor_set_layout::bindings() const
      -> const util::dynamic_array<vk::DescriptorSetLayoutBinding>&
   {
      return m_bindings;
   }

   using builder = descriptor_set_layout::builder;

   builder::builder(vk::Device device, std::shared_ptr<util::logger> p_logger) noexcept :
      m_device{device}, mp_logger{std::move(p_logger)}
   {}
   builder::builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) noexcept :
      m_device{device.logical()}, mp_logger{std::move(p_logger)}
   {}

   auto builder::build() const noexcept -> util::result<descriptor_set_layout>
   {
      return try_wrap([&] {
                return m_device.createDescriptorSetLayoutUnique(
                   {.bindingCount = static_cast<uint32_t>(std::size(m_info.bindings)),
                    .pBindings = m_info.bindings.data()});
             })
         .map_error([]([[maybe_unused]] const auto& err) {
            return to_err_code(descriptor_set_layout_error::failed_to_create_descriptor_set_layout);
         })
         .map([&](auto handle) {
            util::log_info(mp_logger, "[vkn] descriptor set layout created");

            descriptor_set_layout layout;
            layout.m_value = std::move(handle);
            layout.m_bindings = m_info.bindings;

            return layout;
         });
   }

   auto builder::add_binding(const vk::DescriptorSetLayoutBinding& binding) noexcept -> builder&
   {
      m_info.bindings.emplace_back(binding);
      return *this;
   }

   auto builder::set_bindings(
      const util::dynamic_array<vk::DescriptorSetLayoutBinding>& bindings) noexcept -> builder&
   {
      m_info.bindings = bindings;
      return *this;
   }

   struct descriptor_set_layout_error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override
      {
         return "vkn_descriptor_set_layout";
      }

      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return to_string(static_cast<descriptor_set_layout_error>(err));
      }
   };

   inline static const descriptor_set_layout_error_category descriptor_set_layout_category{};

   auto to_string(descriptor_set_layout_error err) -> std::string
   {
      switch (err)
      {
         case descriptor_set_layout_error::failed_to_create_descriptor_set_layout:
            return "failed_to_create_descriptor_set_layout";
         default:
            return "UNKNOWN";
      }
   }
   auto to_err_code(descriptor_set_layout_error err) -> util::error_t
   {
      return {{static_cast<int>(err), descriptor_set_layout_category}};
   }
} // namespace vkn
