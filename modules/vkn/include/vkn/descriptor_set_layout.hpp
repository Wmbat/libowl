#pragma once

#include <vkn/device.hpp>

namespace vkn
{
   enum struct descriptor_set_layout_error
   {
      failed_to_create_descriptor_set_layout
   };

   auto to_string(descriptor_set_layout_error err) -> std::string;
   auto to_err_code(descriptor_set_layout_error err) -> util::error_t;

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
         builder(vk::Device device, util::logger_wrapper logger) noexcept;
         builder(const vkn::device& device, util::logger_wrapper logger) noexcept;

         [[nodiscard]] auto build() noexcept -> util::result<descriptor_set_layout>;

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

         util::logger_wrapper m_logger;
      };
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::descriptor_set_layout_error> : true_type
   {
   };
} // namespace std
