#pragma once

#include <vkn/device.hpp>

namespace vkn
{
   enum struct descriptor_set_layout_error
   {
      failed_to_create_descriptor_set_layout
   };

   auto to_string(descriptor_set_layout_error err) -> std::string;
   auto make_error(descriptor_set_layout_error err, std::error_code ec) -> vkn::error;

   class descriptor_set_layout : public owning_handle<vk::DescriptorSetLayout>
   {
   public:
      [[nodiscard]] auto device() const -> vk::Device;
      [[nodiscard]] auto bindings() const
         -> const util::dynamic_array<vk::DescriptorSetLayoutBinding>&;

   private:
      util::dynamic_array<vk::DescriptorSetLayoutBinding> m_bindings;

   public:
      class builder final
      {
      public:
         builder(vk::Device device, std::shared_ptr<util::logger> p_logger) noexcept;
         builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) noexcept;

         [[nodiscard]] auto build() const noexcept -> vkn::result<descriptor_set_layout>;

         auto add_binding(const vk::DescriptorSetLayoutBinding& binding) noexcept -> builder&;

         auto
         set_bindings(const util::dynamic_array<vk::DescriptorSetLayoutBinding>& bindings) noexcept
            -> builder&;

      private:
         struct build_info
         {
            util::dynamic_array<vk::DescriptorSetLayoutBinding> bindings;
         } m_info;

         vk::Device m_device;

         std::shared_ptr<util::logger> mp_logger;
      };
   };
} // namespace vkn
