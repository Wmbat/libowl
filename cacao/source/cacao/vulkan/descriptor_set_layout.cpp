#include <cacao/vulkan/descriptor_set_layout.hpp>

#include <utility>

namespace vkn
{
   descriptor_set_layout::descriptor_set_layout(descriptor_set_layout_create_info&& info) :
      m_bindings{std::move(info.bindings)},
      m_set_layout{info.device.logical().createDescriptorSetLayoutUnique(
         vk::DescriptorSetLayoutCreateInfo{}
            .setBindingCount(static_cast<std::uint32_t>(std::size(m_bindings)))
            .setPBindings(std::data(m_bindings)))}
   {
      std::string msg = "Descriptor set layout created with:";

      msg += "\n\tbindings = {";
      for (std::size_t i = 0; auto& binding : m_bindings)
      {
         msg += "\n\t\t{ " +
            fmt::format("type = {}, binding = {}, count = {}",
                        vk::to_string(binding.descriptorType), binding.binding,
                        binding.descriptorCount) +
            " }";

         if (i++ != 0)
         {
            msg += ",";
         }
      }
      msg += "\n\t}";

      info.logger.info(msg);
   }

   auto descriptor_set_layout::value() const -> vk::DescriptorSetLayout
   {
      return m_set_layout.get();
   }
   auto descriptor_set_layout::bindings() const -> std::span<const vk::DescriptorSetLayoutBinding>
   {
      return m_bindings;
   }
} // namespace vkn
