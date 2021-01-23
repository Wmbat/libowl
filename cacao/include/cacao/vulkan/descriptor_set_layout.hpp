#pragma once

#include <cacao/device.hpp>

#include <span>

namespace vkn
{
   struct descriptor_set_layout_create_info
   {
      const cacao::device& device;

      crl::dynamic_array<vk::DescriptorSetLayoutBinding> bindings;

      util::logger_wrapper logger;
   };

   class descriptor_set_layout
   {
   public:
      descriptor_set_layout() = default;
      descriptor_set_layout(descriptor_set_layout_create_info&& info);

      [[nodiscard]] auto value() const -> vk::DescriptorSetLayout;
      [[nodiscard]] auto bindings() const -> std::span<const vk::DescriptorSetLayoutBinding>;

   private:
      crl::dynamic_array<vk::DescriptorSetLayoutBinding> m_bindings;

      vk::UniqueDescriptorSetLayout m_set_layout;
   };
} // namespace vkn
