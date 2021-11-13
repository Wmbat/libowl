/**
 * @file libcacao/descriptor_set_layout.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_DESCRIPTOR_SET_LAYOUT_HPP_
#define LIBCACAO_DESCRIPTOR_SET_LAYOUT_HPP_

#include <libcacao/device.hpp>

// Standard Library

#include <span>
#include <vector>

namespace cacao 
{
   struct descriptor_set_layout_create_info
   {
      const cacao::device& device;

      std::vector<vk::DescriptorSetLayoutBinding> bindings;

      mannele::log_ptr logger;
   };

   class descriptor_set_layout
   {
   public:
      descriptor_set_layout() = default;
      explicit descriptor_set_layout(descriptor_set_layout_create_info&& info);

      [[nodiscard]] auto value() const -> vk::DescriptorSetLayout;
      [[nodiscard]] auto bindings() const -> std::span<const vk::DescriptorSetLayoutBinding>;

   private:
      std::vector<vk::DescriptorSetLayoutBinding> m_bindings;

      vk::UniqueDescriptorSetLayout m_set_layout;
   };
} // namespace cacao

#endif // LIBCACAO_DESCRIPTOR_SET_LAYOUT_HPP_
